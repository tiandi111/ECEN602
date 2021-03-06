//
// Created by 田地 on 2021/1/23.
//

#ifndef PROJ1_ECHOSERVER_H
#define PROJ1_ECHOSERVER_H

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

namespace echo {

/*
 * EchoServer implements the network echo protocol.
 * The server can server multiple client concurrently, echo back each line received from clients.
 */
class EchoServer {
  public:
    EchoServer(uint16_t _port, const std::string& _addr, int _backlog);
    ~EchoServer();

    /*
     * start the server in blocking mode.
     */
    void Start();

  private:
    in_port_t port;
    std::string addr;
    int backlog;
    int sockfd;
    sockaddr_in sockAddr;
};

} // namespace echo

#endif //PROJ1_ECHOSERVER_H
