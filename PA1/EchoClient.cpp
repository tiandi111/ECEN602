//
// Created by 田地 on 2021/1/23.
//

#include "EchoClient.h"

#include <stdio.h>

#include <cstring>
#include <iostream>

#include "Reader.h"
#include "utils.h"

void echo::EchoClient::Start() {
    // connect to the server
    sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if (sockfd < 0) {
        throw std::runtime_error(std::string("create socket failed: ") + std::strerror(errno));
    }

    sockAddr.sin_family=AF_INET;
    sockAddr.sin_port=port;
    sockAddr.sin_addr.s_addr=inet_addr(addr.c_str());

    if (connect(sockfd, (sockaddr*)&sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error(std::string("connect server failed: ") + std::strerror(errno));
    }

    ssize_t recv;
    char buf[BUFFER_SIZE];
    SocketReader reader(sockfd);
    // start echoing
    while (true) {
        // get a line from stdin
        if (fgets(buf, BUFFER_SIZE, stdin) != NULL) {

            // do echo once
            ssize_t total;
            if((total = echo::WriteSocket(sockfd, buf, strlen(buf))) > 0) {
                std::cerr<< "write: " << total <<std::endl;
               if ((recv = reader.ReadLine(nullptr, buf, BUFFER_SIZE)) > 0) {
                   std::cerr<< "recv: "<< std::string(buf, recv).c_str()<< "("<<recv<<")" <<std::endl;
               } else {
                   std::cerr<< "read line failed: "<< recv << std::endl;
               }
            } else {
                std::cerr<< "write line failed"<< std::endl;
            }

        } else if (feof(stdin)) { // eof received, close the socket
            echo::CloseSocket(sockfd);
            break;
        } else {
            std::cerr<< "get line failed"<< std::endl;
        }
    }
}