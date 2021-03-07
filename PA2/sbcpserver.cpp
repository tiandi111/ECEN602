//
// Created by Junjie Wang on 2021-03-06.
//

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include "sbcpserver.h"

namespace SBCP {

    SBCPConn::SBCPConn(int _sockfd, uint16_t _timeout, uint16_t _bufSize) : sockfd(_sockfd), timeout(_timeout), bufSize(_bufSize) {
        rbuf = new char[_bufSize];
        wbuf = new char[_bufSize];
    }

    SBCPConn::~SBCPConn() {
        delete[] rbuf;
        delete[] wbuf;
    }

    Message SBCPConn::ReadSBCPMsg() {
        int lenRead = read(sockfd, rbuf, bufSize);
        Message msg;

        if (lenRead == 0) {
            closed = true;
            return msg;
        }

        if (lenRead < MSG_HEADER_SIZE) {
            throw std::runtime_error("Incomplete SBCP message: " + std::string((char *)rbuf, lenRead));
        }

        msg.ReadHeader(rbuf);

        if (MSG_HEADER_SIZE + msg.GetLen() < lenRead) {
            throw std::runtime_error("Incomplete SBCP message: " + std::string((char *)rbuf, lenRead));
        }

        msg.ReadPayload((char *)rbuf + MSG_HEADER_SIZE);

        return msg;
    }

    void SBCPConn::WriteSBCPMsg(Message msg) {
        if (MSG_HEADER_SIZE + msg.GetLen() > bufSize) {
            throw std::runtime_error("SBCP message too long! Max length: " + std::to_string(bufSize));
        }

        msg.WriteBytes(wbuf);

        int len = write(sockfd, wbuf, bufSize);
        if (len < MSG_HEADER_SIZE + msg.GetLen()) {
            throw std::runtime_error("Write incomplete SBCP message");
        }
    }

    bool SBCPConn::isClosed() {
        return closed;
    }


    SBCPServer::SBCPServer(const std::string &_addr, uint16_t _port, int _maxclients) : port(_port), addr(_addr), maxclients(_maxclients) {}

    void SBCPServer::Start() {
        // Ceate socket for listen
        listenSockFD = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
        if (listenSockFD < 0) {
            throw std::runtime_error("create socket failed");
        }

        sockaddr_in sockAddr;
        bzero(&sockAddr, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = htons(port);

        if (inet_pton(AF_INET, addr.c_str(), &sockAddr.sin_addr) <= 0) {
            throw std::runtime_error("wrong IP address: " + addr);
        }

        // Bind
        if (bind(listenSockFD, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
            throw std::runtime_error("bind socket failed");
        }

        // Listen
        if(listen(listenSockFD, backlog) < 0) {
            throw std::runtime_error(std::string("listen socket failed: ") + std::strerror(errno));
        }

        std::cout << "Server started. Listening on " << addr << ":" << port << " ..." << std::endl;

        // Handle zombie child processes
        //signal(SIGCHLD, ChildProcessHandler);


        // Start serving clients
        while (true) {





            int sockAddrLen = sizeof(sockAddr);
            int newSockfd = accept(listenSockFD, (sockaddr *) &sockAddr, (socklen_t * ) & sockAddrLen);
            if (newSockfd < 0) {
                throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
            }

            std::cout << "New connection established." << std::endl;


//
//            // Record new client in
//            sockFDs.push_back(newSockfd);
//
//
            if (fork() == 0) {
                ssize_t recv;

                char buf[1024];

                while (true) {
                    if ((recv = read(newSockfd, buf, 1024)) > 0) {
                        printf("recv: %s(%zd)\n", std::string(buf, recv).c_str(), recv);

                    } else if (recv == 0) { // upon socket close, child process exit
                        std::cout << "Child process exits" << std::endl;
                        exit(0);
                    } else {
                        std::cerr << "read failed: errno = " << errno << std::endl;
                    }
                }
            }
        }

    }
}