//
// Created by 田地 on 2021/1/23.
//

#include "EchoServer.h"

#include <stdexcept>

#include "Reader.h"
#include "utils.h"

void echo::EchoServer::Start() {
    //
    sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if(sockfd < 0) {
        throw std::runtime_error(std::string("create socket failed: ") + std::strerror(errno));
    }

    sockAddr.sin_family=AF_INET;
    sockAddr.sin_port=port;
    sockAddr.sin_addr.s_addr=inet_addr(addr.c_str());

    if(bind(sockfd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error(std::string("bind socket failed: ") + std::strerror(errno));
    }

    if(listen(sockfd, backlog) < 0) {
        throw std::runtime_error(std::string("listen socket failed: ") + std::strerror(errno));
    }

    // start serving clients
    while (true) {
        int sockAddrLen = sizeof(sockAddr);
        int newSockfd = accept(sockfd, (sockaddr*)&sockAddr, (socklen_t*)&sockAddrLen);
        if (newSockfd < 0) {
            throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
        }

        if (fork() == 0) {
            ssize_t recv;
            SocketReader reader(newSockfd);
            char buf[BUFFER_SIZE];
            while (true) {
                if ((recv = reader.ReadLine(nullptr, buf, BUFFER_SIZE)) > 0) {
                    printf("recv: %s(%zd)\n", std::string(buf, recv).c_str(), recv);

                    if (echo::WriteSocket(newSockfd, buf, recv) < 0) {
                        printf("echo back failed\n");
                    } else {
                        printf("sent: %s\n", std::string(buf, recv).c_str());
                    }
                } else {
                    std::cerr<< "read line failed: "<< recv << std::endl;
                }
            }
        }
    }
}