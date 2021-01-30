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

class EchoServer {
  public:
    EchoServer(int _port, const std::string& _addr, int _backlog) :
            port(_port), addr(_addr), backlog(_backlog) {

        sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
        if(sockfd < 0) {
            throw std::runtime_error("create socket failed");
        }

        sockAddr.sin_family=AF_INET;
        sockAddr.sin_port=port;
        sockAddr.sin_addr.s_addr=inet_addr(addr.c_str());

        if(bind(sockfd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0) {
            throw std::runtime_error("bind socket failed");
        }

        if(listen(sockfd, backlog) < 0) {
            throw std::runtime_error("listen socket failed");
        }
    }
    ~EchoServer() {
        close(sockfd);
    }
    void Start();
    void Stop();

  private:
    in_port_t port;
    std::string addr;
    int backlog;
    int sockfd;
    sockaddr_in sockAddr;
};

} // namespace echo

#endif //PROJ1_ECHOSERVER_H
