#include "echoserver.h"

#include <stdexcept>
#include <string>
#include <iostream>


int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cout<< "usage: server <PORT>" <<std::endl;
        return -1;
    }
    try {
        int port = std::stoi(argv[1]);
        echo::EchoServer server = echo::EchoServer(port, "0.0.0.0", 10);
        server.Start();
    } catch (std::exception const &err) {
        std::cout<< err.what() <<std::endl;
    }
    return 0;
}
