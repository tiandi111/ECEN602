//
// Created by 田地 on 2021/1/23.
//

#ifndef PROJ1_ECHOCLIENT_H
#define PROJ1_ECHOCLIENT_H

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

namespace echo {

/*
 * EchoClient implements the network echo protocol.
 * Once started, the client read a line of characters from stdin, send it to the target server
 * and read back and print the line.
 */
class EchoClient {
  public:
    EchoClient(int _port, const std::string& _addr) : port(_port), addr(_addr){}
    ~EchoClient() {
        close(sockfd);
    }
    /*
     * start the client in blocking mode.
     */
    void Start();

  private:
    in_port_t port;
    std::string addr;
    int sockfd;
    sockaddr_in sockAddr;
};

} // namespace echo

#endif //PROJ1_ECHOCLIENT_H
