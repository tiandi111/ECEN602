//
// Created by Junjie Wang on 2021-03-07.
//


#include <iostream>

#include "sbcpserver.h"

using namespace SBCP;
using namespace std;

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "Not enough arguments. Please specify IP, port, and maximum number of clients." << endl;
        return -1;
    }

    string ip = argv[1];
    int port = stoi(argv[2]);
    int maxClients = stoi(argv[3]);

    SBCPServer server = SBCPServer(ip, port, maxClients);

    try{
        server.Start();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}