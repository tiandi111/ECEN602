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

class EchoClient {
  public:
    EchoClient(int _port, const std::string& _addr) :
            port(_port), addr(_addr){

        sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
        if (sockfd < 0) {
            throw std::runtime_error("create socket failed");
        }

        sockAddr.sin_family=AF_INET;
        sockAddr.sin_port=port;
        sockAddr.sin_addr.s_addr=inet_addr(addr.c_str());

        if (connect(sockfd, (sockaddr*)&sockAddr, sizeof(sockAddr))<0) {
            throw std::runtime_error("connect server failed");
        }
    }
    ~EchoClient() {
        close(sockfd);
    }
    void Start();
    void Stop();

  private:
    in_port_t port;
    std::string addr;
    int sockfd;
    sockaddr_in sockAddr;
};

} // namespace echo

#endif //PROJ1_ECHOCLIENT_H
