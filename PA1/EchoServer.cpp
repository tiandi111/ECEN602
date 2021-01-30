//
// Created by 田地 on 2021/1/23.
//

#include "EchoServer.h"

#include <stdexcept>

#include "reader.h"
#include "utils.h"

void echo::EchoServer::Start() {
    while (true) {
        int sockAddrLen = sizeof(sockAddr);
        int newSockfd = accept(sockfd, (sockaddr*)&sockAddr, (socklen_t*)&sockAddrLen);
        if (newSockfd < 0) {
            throw std::runtime_error("accept failed");
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

void echo::EchoServer::Stop() {

}