//
// Created by 田地 on 2021/1/23.
//

#include "EchoClient.h"

#include <stdio.h>

#include <cstring>
#include <iostream>

#include "iosocket.h"


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
    char buf[BUFFER_SIZE];
    IOSocket iosocket(sockfd);
    // start echoing
    while (true) {
        // get a line from stdin
        if (fgets(buf, BUFFER_SIZE, stdin) != NULL) {

            // do echo once
            ssize_t total;
            if((total = iosocket.Write(sockfd, buf, strlen(buf))) > 0) {
                std::cerr<< "write: " << total <<std::endl;
               if ((recv = iosocket.ReadLine(nullptr, buf, BUFFER_SIZE)) > 0) {
                   std::cerr<< "recv: "<< std::string(buf, recv).c_str()<< "("<<recv<<")" <<std::endl;
               } else {
                   std::cerr<< "read line failed: "<< recv << std::endl;
               }
            } else {
                std::cerr<< "write line failed"<< std::endl;
            }

        } else if (feof(stdin)) { // eof received, close the socket
            echo::IOSocket::CloseSocket(sockfd);
            break;
        } else {
            std::cerr<< "get line failed"<< std::endl;
        }
    }
}