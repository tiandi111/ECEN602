//
// Created by 田地 on 2021/2/28.
//

#include <iostream>
#include <thread>
#include "sbcpclient.h"

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr<< "usage: client <USERNAME> <ADDR> <PORT>"<< std::endl;
        return 1;
    }
    try {
        int port = std::stoi(argv[3]);
        SBCP::SBCPClient client(argv[1], argv[2], port);
        return client.Start();
    } catch (std::exception const& err) {
        std::cerr<< err.what() <<std::endl;
    }
    return 1;
}
