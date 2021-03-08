//
// Created by 田地 on 2021/2/28.
//

#include <stdexcept>
#include <string>
#include <iostream>
#include "sbcpserver.h"


int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr<< "usage: server <PORT> <MaxClient=3>" <<std::endl;
        return -1;
    }
    try {
        uint32_t maxClient = 3;
        int port = std::stoi(argv[1]);
        if (argc > 2) {
            maxClient = std::stoul(argv[2]);
        }
        SBCP::SBCPServer server(port, "0.0.0.0", maxClient);
        server.Start();
    } catch (std::exception const &err) {
        std::cerr<< err.what() <<std::endl;
    }
    return 0;
}
