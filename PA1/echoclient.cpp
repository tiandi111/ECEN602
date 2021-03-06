//
// Created by 田地 on 2021/1/23.
//

#include "echoclient.h"

#include <stdio.h>

#include <cstring>
#include <iostream>

#include "iosocket.h"

#define CLI_BUFFER_SIZE 1024

echo::EchoClient::EchoClient(const std::string &_addr, uint16_t _port) :
        port(_port), addr(_addr) {

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error(std::string("create socket failed: ") + std::strerror(errno));
    }

    bzero(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(_port);

    if (inet_pton(AF_INET, _addr.c_str(), &sockAddr.sin_addr) <= 0) {
        throw std::runtime_error("Wrong IP address: " + _addr);
    }

    if (connect(sockfd, (sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error(std::string("connect server failed: ") + std::strerror(errno));
    }
}

echo::EchoClient::~EchoClient() {
    close(sockfd);
}

void echo::EchoClient::Start() {
    ssize_t recv;
    char buf[CLI_BUFFER_SIZE];
    IOSocket iosocket(sockfd);
    // start echoing
    while (true) {
        // get a line from stdin
        if (fgets(buf, CLI_BUFFER_SIZE, stdin) != NULL) {

            // std::cout << "Buf content: " << buf << "length: " << strlen(buf) << std::endl;
            int sendLen = strlen(buf);
            // guarantee that the line ends with '\0' or '\n' or EOF
            if(buf[sendLen-1] != '\n' && buf[sendLen-1] != '\0' && buf[sendLen-1] != EOF)
                ++sendLen;
            // do echo once
            ssize_t total;
            if((total = iosocket.Write(sockfd, buf, sendLen)) > 0) {
                std::cerr<< "write: " << total <<std::endl;
               if ((recv = iosocket.ReadLine(nullptr, buf, CLI_BUFFER_SIZE)) > 0) {
                   std::cerr<< "recv: "<< std::string(buf, recv).c_str()<< "("<<recv<<")" <<std::endl;
               } else {
                   std::cerr<< "read line failed: "<< recv << std::endl;
               }
            } else {
                std::cerr<< "write line failed"<< std::endl;
            }

        } else if (feof(stdin)) { // eof received, close the socket
            std::cout<< "Get EOF"<< std::endl;
            echo::IOSocket::CloseSocket(sockfd);
            break;
        } else {
            std::cerr<< "get line failed"<< std::endl;
        }
    }
}