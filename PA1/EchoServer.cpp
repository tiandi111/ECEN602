//
// Created by 田地 on 2021/1/23.
//

#include "EchoServer.h"

#include <stdexcept>
#include <csignal>

#include "iosocket.h"

echo::EchoServer::EchoServer(uint16_t _port, const std::string &_addr, int _backlog) :
        port(_port), addr(_addr), backlog(_backlog) {

    sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if (sockfd < 0) {
        throw std::runtime_error("create socket failed");
    }

    bzero(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, _addr.c_str(), &sockAddr.sin_addr) <= 0) {
        throw std::runtime_error("wrong IP address: " + _addr);
    }

    if (bind(sockfd, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error("bind socket failed");
    }
}

echo::EchoServer::~EchoServer() {
    close(sockfd);
}


void ChildProcessHandler(int sig) {
    wait(NULL);
}

void echo::EchoServer::Start() {
    if(listen(sockfd, backlog) < 0) {
        throw std::runtime_error(std::string("listen socket failed: ") + std::strerror(errno));
    }

    signal(SIGCHLD, ChildProcessHandler); // handle zombie child process

    // start serving clients
    while (true) {
        int sockAddrLen = sizeof(sockAddr);
        int newSockfd = accept(sockfd, (sockaddr*)&sockAddr, (socklen_t*)&sockAddrLen);
        if (newSockfd < 0) {
            throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
        }

        if (fork() == 0) {
            ssize_t recv;
            IOSocket iosocket(newSockfd);
            char buf[BUFFER_SIZE];
            while (true) {
                if ((recv = iosocket.ReadLine(nullptr, buf, BUFFER_SIZE)) > 0) {
                    printf("recv: %s(%zd)\n", std::string(buf, recv).c_str(), recv);

                    if (iosocket.Write(newSockfd, buf, recv) < 0) {
                        printf("echo back failed\n");
                    } else {
                        printf("sent: %s\n", std::string(buf, recv).c_str());
                    }

                } else if (iosocket.SocketClosed()) { // upon socket close, child process exit
                    std::cout << "Child process exits" << std::endl;
                    exit(0);
                } else {
                    std::cerr<< "read line failed: "<< recv << std::endl;
                }
            }
        }
    }
}