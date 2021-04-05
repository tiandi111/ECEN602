//
// Created by 田地 on 2021/3/14.
//

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "tftpsession.h"
#include "utils.h"

using namespace std::chrono;

using namespace TFTP::Session;

const std::string netascii = "netascii";
const std::string octet = "octet";
const std::string test = "test";

/// ===========================================
///                 DaemonSession
/// ===========================================
DaemonSession::DaemonSession(std::string _addr, int _port, int _backlog)
: addr(std::move(_addr)),
  port(_port),
  backlog(_backlog) {}

DaemonSession::~DaemonSession() {
    if (sockfd > 0) {
        close(sockfd);
    }
}

void DaemonSession::Init() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // use udp
    if (sockfd < 0) {
        throw std::runtime_error(std::string("socket: ") + strerror(errno));
    }

    bzero(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, addr.c_str(), &sockAddr.sin_addr) <= 0) {
        throw std::runtime_error(std::string("inet_pton: ") + strerror(errno));
    }

    if (bind(sockfd, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error(std::string("bind: ") + strerror(errno));
    }
}

UniqueRequestSession DaemonSession::AcceptRequest(milliseconds timeout) {
    sockaddr srcAddr = {};
    socklen_t addrLen = sizeof(srcAddr);

    int ret = TEMP_FAILURE_RETRY(recvfrom(
        sockfd,
        buf,
        sizeof(buf),
        0,
        &srcAddr,
        &addrLen));
    if (ret <= 0) {
        throw std::runtime_error(std::string("recvfrom: ") + strerror(errno));
    }
    if (ret < 2) {
        throw std::runtime_error(std::string("recvfrom: too few data, discard packet"));
    }

    int16_t opcode = ntohs(*(int16_t *)(buf));
    // todo: handle corner cases
    uint32_t max = sizeof(buf) - 2;
    if (strnlen(buf + 2, max) >= max) {
        throw std::runtime_error(std::string("discard truncated packet"));
    }
    std::string path (buf + 2);
    max = sizeof(buf) - 3 - path.size();
    if (strnlen(buf + 3 + path.size(), max) >= max) {
        throw std::runtime_error(std::string("discard truncated packet"));
    }
    std::string mode (buf + 3 + path.size());

    switch (opcode) {
        case RRQ:
            return RRQSession::Create(srcAddr, path, mode, timeout);
        default:
            throw std::runtime_error(std::string("invalid opcode: ") + std::to_string(opcode));
    }
}

const FileDescriptor DaemonSession::Socket() const {
    return sockfd;
}

UniqueDaemonSession DaemonSession::Create(std::string _addr, int _port, int _backlog) {
    return UniqueDaemonSession (new DaemonSession(std::move(_addr), _port, _backlog));
}

/// ===========================================
///               RequestSession
/// ===========================================
RequestSession::RequestSession(sockaddr _destAddr, std::string _path, std::string _mode, milliseconds _timeout)
: destAddr(_destAddr),
  path(std::move(_path)),
  mode(std::move(_mode)),
  timeout(_timeout),
  closed(false) {
    if (mode != netascii && mode != octet && mode != test) {
        throw std::runtime_error("unsupported mode: " + mode);
    }
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd <= 0) {
        throw std::runtime_error(std::string("socket: ") + strerror(errno));
    }
}

RequestSession::~RequestSession() noexcept {
    cleanup();
}

void RequestSession::Terminate() {
    closed = true;
}

bool RequestSession::Terminated() {
    return closed;
}

const std::string RequestSession::Path() const {
    return path;
}

const std::string RequestSession::Mode() const {
    return mode;
}

const FileDescriptor RequestSession::Socket() const {
    return sockfd;
}

const std::string RequestSession::ErrMsg() {
    return errmsg;
}

void RequestSession::SendErrorPacket(ErrorCode ec, std::string msg) {
    if ( sendErrorPacket(ec, msg) < 0 ) {
        throw std::runtime_error(std::string("sendErrorPacket: ") + strerror(errno));
    }
}

const std::string RequestSession::ID() {
    return id;
}

void RequestSession::cleanup() {
    if (sockfd > 0) close(sockfd);
}

int RequestSession::sendErrorPacket(ErrorCode ec, std::string msg) {
    char buf[5 + msg.size()];
    *(uint16_t *)(buf) = htons(ERROR);
    // note: mac tftp client doesn't do byte order conversion,
    //       so we avoid it here too
    *(uint16_t *)(buf + 2) = ec; //htons(ec);
    strcpy(buf + 4, msg.data());
    return TEMP_FAILURE_RETRY(sendto(
        sockfd,
        buf,
        sizeof(buf),
        0,
        &destAddr,
        sizeof(destAddr)
    ));
}

/// ===========================================
///                 RRQSession
/// ===========================================
UniqueRRQSession RRQSession::Create(
    sockaddr destAddr,
    std::string path,
    std::string mode,
    milliseconds timeout) {
    return UniqueRRQSession (new RRQSession(destAddr, path, mode, timeout)) ;
}

RRQSession::RRQSession(
    sockaddr _destAddr,
    std::string _path,
    std::string _mode,
    milliseconds _timeout)
: RequestSession(_destAddr, std::move(_path), std::move(_mode), _timeout),
  curblk(0),
  acked(0),
  tryNum(0),
  dataIn(0),
  dupAck(false),
  eof(false),
  lastSentTime(steady_clock::now()) {
    id = "RRQ_" + std::to_string(Socket()) + "_" + std::to_string(std::time(nullptr));
    try {
        reader = IO::UniqueIFileReaderFactory(mode, path);
    } catch (std::exception& e) {
        sendErrorPacket(FileNotFound, e.what());
        cleanup();
        throw;
    }
    if ( !reader ) {
        cleanup();
        throw std::runtime_error("unsupported reader type" + mode);
    }
}

void RRQSession::Proceeds(bool readReady, bool writeReady) {
    if ( Terminated() ) {
        return ;
    }
    if ( readReady ) {
        recvAck();
    }
    if ( writeReady ) {
        sendData();
    }
}

const std::string RRQSession::TypeStr() {
    return "RRQ";
}

void RRQSession::recvAck() {
    if ( closed ) return;

    sockaddr srcAddr {};
    socklen_t srcAddrLen = sizeof(srcAddr);
    int ret = TEMP_FAILURE_RETRY(
        recvfrom(
            sockfd,
            recvBuf,
            sizeof(recvBuf),
            0,
            &srcAddr,
            &srcAddrLen));
    if (ret <= 0) {
        throw std::runtime_error(std::string ("recvfrom: ") + strerror(errno));
    }

    if (CmpSockaddr(&srcAddr, &destAddr) != 0) {
        // todo: send error packet
        return ;
    }

    Opcode opcode = Opcode(ntohs(*(uint16_t *) recvBuf));

    switch ( opcode ) {
        case ACK: {
            int blk = ntohs(*(uint16_t *) (recvBuf + 2));
            DEBUG(id, "recv ack: " + std::to_string(blk));
            if (blk == curblk && acked + 1 == curblk) {
                acked++;
            }
            if (blk == acked) {
                dupAck = true;
                closed = eof;
            }
            break;
        }
        case ERROR:
            errmsg = std::string(recvBuf + 4);
            closed = true;
            DEBUG(id, "recv error: " + errmsg);
            break;
        default:
            throw std::runtime_error(std::string("unexpected opcode: ") + std::to_string(opcode));
    }
}

void RRQSession::sendData() {
    if ( closed ) return;

    if (acked == curblk) {
        curblk ++;
        tryNum = 0;
        *(uint16_t *) dataBuf = htons(3);
        *(uint16_t *) (dataBuf + 2) = htons(curblk);
        dataIn = 4;
    }

    while ( !dataReady() ) {
        int ret = reader->Read(dataBuf + dataIn, sizeof(dataBuf) - dataIn);
        if (ret < 0) {
            throw std::runtime_error(std::string("Read: ") + strerror(errno));
        }
        if (ret == 0) {
            eof = true;
        }
        dataIn += ret;
    }

    if ( dataReady() && ( tryNum == 0 || dupAck || ackTimeout() ) ) {
        if (tryNum == MAX_RETRY) {
            closed = true;
            errmsg = "max retry exceeded";
            return;
        }
        int ret = TEMP_FAILURE_RETRY(
            sendto(
                sockfd,
                dataBuf,
                dataIn,
                0,
                &destAddr,
                sizeof(destAddr)));
        if (ret <= 0) {
            throw std::runtime_error(std::string("sendto: ") + strerror(errno));
        }
        DEBUG(id, "send data block: " + std::to_string(curblk));
        tryNum ++;
        lastSentTime = steady_clock::now();
        dupAck = false;
    }
}

bool RRQSession::dataReady() {
    return eof || dataIn == sizeof(dataBuf);
}

bool RRQSession::ackTimeout() {
    return acked < curblk && steady_clock::now() - lastSentTime > timeout;
}

