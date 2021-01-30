//
// Created by 田地 on 2021/1/23.
//

#include "EchoClient.h"

#include <stdio.h>

#include <cstring>
#include <iostream>

#include "reader.h"
#include "utils.h"

void echo::EchoClient::Start() {
    ssize_t received;
    char buf[BUFFER_SIZE];
    SocketReader reader(sockfd);
    while (true) {
        // get lines from stdin
        if (fgets(buf, BUFFER_SIZE, stdin) != NULL) {

            // do echo once
            ssize_t total;
            if((total = echo::WriteSocket(sockfd, buf, strlen(buf))) > 0) {
                std::cerr<< "write: " << total <<std::endl;
               if ((received = reader.ReadLine(nullptr, buf, BUFFER_SIZE)) > 0) {
                   std::cerr<< "recv: "<< std::string(buf, received).c_str()<< "("<<received<<")" <<std::endl;
               } else {
                   std::cerr<< "read line failed: "<< received << std::endl;
               }
            } else {
                std::cerr<< "write line failed"<< std::endl;
            }

        } else {
            std::cerr<< "get line failed"<< std::endl;
        }
    }
}

void echo::EchoClient::Stop() {

}