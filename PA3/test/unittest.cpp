//
// Created by 田地 on 2021/3/24.
//

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <stdio.h>
#include <stdlib.h>

#define private public // to access private members for testing purpose
#include "../io.h"
#include "../tftpsession.h"
#include "unittest.h"

using namespace std::chrono;

std::string TestMode = "test";

// ===========================================
//            Test Helper Functions
// ===========================================
void WriteRRQPacket(char* buf, std::string filename, std::string mode) {
    *(uint16_t *) buf = htons(1);
    memcpy(buf + 2, filename.data(), filename.size());
    buf[2 + filename.size()] = '\0';
    memcpy(buf + 3 + filename.size(), mode.data(), mode.size());
    buf[2 + filename.size() + 1 + mode.size()] = '\0';
}

void WriteWRQPacket(int fd, std::string filename, std::string mode) {

}

void WriteAckPacket(char* dst, int blkno) {
    *(uint16_t *) dst = htons(4);
    *(uint16_t *) (dst + 2) = htons(blkno);
}

void WriteErrPacket(char* dst, TFTP::Session::RequestSession::ErrorCode ec, std::string msg) {
    *(uint16_t *) dst = htons(5);
    *(uint16_t *) (dst + 2) = htons(ec);
    strcpy(dst + 4, msg.data());
}

void CreateTempFile(std::string filename, int size) {
    std::fstream tempFile(filename, std::ios::binary | std::ios::out);
    tempFile.seekp(size - 1);
    tempFile.write("", 1);
}

// ===========================================
//               Test Functions
// ===========================================
void TestDaemonSession() {
    TFTP::Session::DaemonSession daemon ("", 0, 0);
    daemon.Init();

    std::string filepath = "test_file";

    { // Test 1: normal case
        WriteRRQPacket(daemon.buf, filepath, TestMode);
        auto sess = daemon.AcceptRequest(milliseconds(10));
        assert(dynamic_cast<TFTP::Session::RRQSession *>(sess.get()));
        assert(sess->Path() == filepath && sess->Mode() == TestMode);
    }

    { // Test 2: filepath too long
        *(uint16_t *) daemon.buf = htons(1);
        memset(daemon.buf + 2, '*', sizeof(daemon.buf) - 2);
        try {
            auto sess = daemon.AcceptRequest(milliseconds(10));
            assert(("truncated packet should be discarded and throw exception", false));
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void TestRRQSession() {
    { // Test Constructor
        TFTP::Session::RRQSession rrq({}, "", TestMode, milliseconds(1));
        assert(rrq.curblk == 0 && rrq.acked == 0 && rrq.tryNum == 0 && rrq.dataIn == 0 && !rrq.eof);
    }

    { // Test recvAck()

        { // Test 1: receive unexpected packet type
            TFTP::Session::RRQSession rrq({}, "", TestMode, milliseconds(1));
            try {
                *(uint16_t *) rrq.recvBuf = htons(999);
                rrq.recvAck();
                assert(("exception should be thrown when unexpected packet type received", false));
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }

        { // Test 2: receive ERROR packet
            TFTP::Session::RRQSession rrq({}, "", TestMode, milliseconds(1));
            WriteErrPacket(rrq.recvBuf, TFTP::Session::RequestSession::ErrorCode::Undefined, "test error");
            rrq.recvAck();
            assert(rrq.ErrMsg() == "test error" && rrq.Terminated());
        }

        { // Test 3: receive ACK packet
            TFTP::Session::RRQSession rrq({}, "", TestMode, milliseconds(1));

            WriteAckPacket(rrq.recvBuf, 1);
            rrq.recvAck();
            assert(("receive unexpected ACK", rrq.acked == 0));

            rrq.acked = 0; rrq.curblk = 1;
            WriteAckPacket(rrq.recvBuf, 1);
            rrq.recvAck();
            assert(("receive expected ACK", rrq.acked == 1));

            rrq.acked = 0; rrq.curblk = 1;
            WriteAckPacket(rrq.recvBuf, 0);
            rrq.recvAck();
            assert(("receive duplicate ACK", rrq.acked == 0 && rrq.dupAck));
        }
    }

    { // Test sendData()

        { // Test 1: send data packet
            TFTP::Session::RRQSession rrq({}, "", TestMode, milliseconds(1));

            rrq.sendData();
            assert(("first send", rrq.curblk == 1 && rrq.tryNum == 1));
            assert(("data header", ntohs(*(uint16_t *) rrq.dataBuf) == TFTP::Session::DATA));

            rrq.sendData();
            assert(("no resend before ACK", rrq.curblk == 1 && rrq.tryNum == 1));

            rrq.dupAck = true;
            rrq.sendData();
            assert(("resend immediately after duplicate ACK", rrq.curblk == 1 && rrq.tryNum == 2 && !rrq.dupAck));

            sleep(1);
            rrq.sendData();
            assert(("resend if timeout", rrq.curblk == 1 && rrq.tryNum == 3));

            rrq.tryNum = 10;
            sleep(1);
            rrq.sendData();
            assert(("max retry exceeded", rrq.Terminated() && rrq.ErrMsg() == "max retry exceeded"));
        }

    }

    { // Test sendData() recvAck interactions
        TFTP::Session::RRQSession rrq({}, "", TestMode, milliseconds(1));

        // a successful send-ack round
        rrq.sendData();
        WriteAckPacket(rrq.recvBuf, rrq.curblk);
        rrq.recvAck();
        assert(rrq.curblk == 1);

        rrq.sendData();

        // a dup ack round
        WriteAckPacket(rrq.recvBuf, rrq.curblk - 1); // a dup ack
        rrq.recvAck();
        rrq.sendData();
        assert(rrq.curblk == 2 && rrq.tryNum == 2);

        // a timeout send
        sleep(1);
        rrq.sendData();
        assert(rrq.curblk == 2 && rrq.tryNum == 3);

        // another successful send-ack round
        WriteAckPacket(rrq.recvBuf, rrq.curblk);
        rrq.recvAck();
        rrq.sendData();
    }
}

void TestReader() {
    using namespace TFTP::IO;
#define CreateTestFile(content)\
    ({  std::string __filename = "test_file_" + std::to_string(time(nullptr));\
        FILE* __tfile = fopen(__filename.c_str(), "w");\
        fwrite(content, 1, sizeof(content), __tfile);\
        fflush(__tfile);\
        fclose(__tfile);\
        __filename;})\

#define CleanTestFile(fname)\
    remove(fname.c_str());

    { // Test 1: replace CR with CR NULL, LF with CR LF
        char content[] = {'a', '\r', 'b', '\n', 'c'};
        char netasciiEncoded[] = {'a', '\r', '\0', 'b', '\r', '\n', 'c'};
        std::string fname = CreateTestFile(content);

        { // NetasciiFileReader
            NetasciiFileReader reader(fname);
            char buf[sizeof(netasciiEncoded)];
            assert(reader.Read(buf, sizeof(buf)) == sizeof(buf));
            assert(std::equal(netasciiEncoded, netasciiEncoded + sizeof(netasciiEncoded), buf));
            assert(("test eof", reader.Read(buf, sizeof(buf)) == 0));
        }

        { // OctetFileReader
            OctetFileReader reader(fname);
            char buf[sizeof(content)];
            assert(reader.Read(buf, sizeof(buf)) == sizeof(buf));
            assert(std::equal(content, content + sizeof(content), buf));
            assert(("test eof", reader.Read(buf, sizeof(buf)) == 0));
        }

        CleanTestFile(fname);
    }

    { // Test 2: separate reads
        char content[] = {'a', '\r', 'b', '\n', 'c', '\r', '\n', '\r', '\n'};
        char netasciiEncoded[] = {'a', '\r', '\0', 'b', '\r', '\n', 'c', '\r', '\0', '\r', '\n', '\r', '\0', '\r', '\n'};
        std::string fname = CreateTestFile(content);

        { // NetasciiFileReader
            NetasciiFileReader reader(fname);
            char buf[sizeof(netasciiEncoded)];
            for (int i = 0; i < sizeof(netasciiEncoded); ++i) {
                assert(reader.Read(buf + i, 1) == 1);
            }
            assert(std::equal(netasciiEncoded, netasciiEncoded + sizeof(netasciiEncoded), buf));
            assert(reader.Read(buf, sizeof(buf)) == 0);
        }

        { // OctetFileReader
            OctetFileReader reader(fname);
            char buf[sizeof(content)];
            for (int i = 0; i < sizeof(content); ++i) {
                assert(reader.Read(buf + i, 1) > 0);
            }
            assert(std::equal(content, content + sizeof(content), buf));
            assert(reader.Read(buf, sizeof(buf)) == 0);
        }

        CleanTestFile(fname);
    }
}

int main() {
    TEST_START_TRACE("TestDaemonSession");
    TestDaemonSession();
    TEST_END_TRACE("TestDaemonSession");

    TEST_START_TRACE("TestRRQSession");
    TestRRQSession();
    TEST_END_TRACE("TestRRQSession");

    TEST_START_TRACE("TestReader");
    TestReader();
    TEST_END_TRACE("TestReader");
}