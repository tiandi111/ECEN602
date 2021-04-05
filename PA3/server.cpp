//
// Created by 田地 on 2021/4/4.
//

#include <chrono>
#include <iostream>
#include <string>

#include "tftpserver.h"

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr<< "usage: server <PORT>" <<std::endl;
        return -1;
    }
    try {
        int port = std::stoi(argv[1], nullptr);
        TFTP::PollServer server ({
            "0.0.0.0",
            port,
            10,
            std::chrono::milliseconds (500),
        });
        server.Start();
    } catch (std::exception const &err) {
        std::cerr<< err.what() <<std::endl;
    }
    return 0;
}