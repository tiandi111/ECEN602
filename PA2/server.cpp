//
// Created by 田地 on 2021/2/28.
//

#include <stdexcept>
#include <string>
#include <iostream>
#include "sbcpserver.h"


int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr<< "usage: server <PORT>" <<std::endl;
        return -1;
    }
    try {
        int port = std::stoi(argv[1]);
        SBCP::SBCPServer server(port, "0.0.0.0", 10);
        server.Start();
    } catch (std::exception const &err) {
        std::cerr<< err.what() <<std::endl;
    }
    return 0;
}
