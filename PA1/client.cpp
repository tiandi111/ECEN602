#include "EchoClient.h"

#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr<< "usage: client <ADDR> <PORT>"<< std::endl;
        return -1;
    }
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (std::exception const& err) {
        std::cerr<< err.what() <<std::endl;
    }
    try {
        echo::EchoClient client = echo::EchoClient(argv[1], port);
        client.Start();
    } catch (std::exception const& err) {
        std::cerr<< err.what() <<std::endl;
    }
}
