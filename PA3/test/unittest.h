//
// Created by 田地 on 2021/3/24.
//

#ifndef PA3_UNITTEST_H
#define PA3_UNITTEST_H

#include <string>

#define TEST_START_TRACE(name)\
    std::cout<< "Run Test: " << name << "..." <<std::endl;\
    std::cout.flush();\

#define TEST_END_TRACE(name)\
    std::cout<< "Pass Test: " << name << "." <<std::endl;\
    std::cout.flush();\

// Test Utils
void WriteRRQPacket(char* buf, std::string filename, std::string mode) ;
void WriteWRQPacket(int fd, std::string filename, std::string mode) ;
void WriteAckPacket(char* dst, int blkno) ;
void WriteErrPacket(char* dst, TFTP::Session::RequestSession::ErrorCode ec, std::string msg) ;
void ReadDataPacket(int fd, char *buf);
void CreateTempFile(std::string filename, int size) ;

// Unit tests
void TestDaemonSession() ;
void TestRRQSession() ;
void TestReader() ;

#endif //PA3_UNITTEST_H
