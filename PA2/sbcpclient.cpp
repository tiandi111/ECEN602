//
// Created by 田地 on 2021/2/28.
//

#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "io.h"
#include "sbcpclient.h"
#include "protocol.h"

SBCP::SBCPClient::SBCPClient(std::string _username, std::string _addr, uint16_t _port)
    : username(std::move(_username)),
      port(_port),
      addr(std::move(_addr)),
      closed(false)
      {}

SBCP::SBCPClient::~SBCPClient() {
    close(sockfd);
}

void SBCP::SBCPClient::Init() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error(std::string("socket: ") + strerror(errno));
    }

    bzero(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, addr.c_str(), &sockAddr.sin_addr) <= 0) {
        throw std::runtime_error("wrong IP address: " + addr);
    }

    if (connect(sockfd, (sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error(std::string("connect: ") + strerror(errno));
    }

    Message joinMsg = NewJoinMessage(username);
    char buf[joinMsg.Size()];
    joinMsg.WriteBytes(buf);
    if(send(sockfd, buf, joinMsg.Size(), 0) < 0) {
        throw std::runtime_error(std::string("send: ") + strerror(errno));
    }

    std::cout<< "Connected to server " << addr << ":" << port <<std::endl;
}

int SBCP::SBCPClient::WaitEvent() {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sockfd, &readfds);
    int ret = select(std::max(sockfd, STDIN_FILENO) + 1, &readfds , NULL , NULL , NULL);
    if (ret < 0) {
        throw std::runtime_error(std::string("select: ") + strerror(errno));
    }
    return ret;
}

void SBCP::SBCPClient::ForwardMessages() {
    char buf[CLT_BUFFER_SIZE];
    if (FD_ISSET(STDIN_FILENO, &readfds) && fgets(buf, CLT_BUFFER_SIZE, stdin) != NULL) {
        Send(buf, strlen(buf));
    }
}

void SBCP::SBCPClient::RecvMessages() {
    if (FD_ISSET(sockfd, &readfds)) {
        Message msg;
        int code = SBCP::ReadMessage(msg,  sockfd);
        if (code == 0) {
            closed = true;
            return;
        }
        if (code < 0 ) {
            throw std::runtime_error(std::string("ReadMessage: ") + strerror(errno));
        }
        if (msg.GetType() == Message::FWD) {
            auto username = msg.GetAttrMap().at(Attribute::Username).GetPayloadString();
            auto info = msg.GetAttrMap().at(Attribute::Message).GetPayloadString();
            std::cout<< username << ": " << info; std::cout.flush();
        }
    }
}

void SBCP::SBCPClient::Send(void* ptr, size_t len) {
    Message msg =  NewSendMessage(std::string((char *) ptr, len));
    char buf[msg.Size()];
    msg.WriteBytes(buf);
    if (SBCP::writelen(sockfd, buf, msg.Size()) < 0) {
        throw std::runtime_error(std::string("writelen: ") + strerror(errno));
    }
}

bool SBCP::SBCPClient::IsClosed() {
    return closed;
}

int SBCP::SBCPClient::Start() {
    Init();

    while (true) {

        std::cout<< "> "; std::cout.flush();

        try {
            WaitEvent();

            ForwardMessages();

            RecvMessages();

            if (IsClosed()) {
                std::cout<< "Connection broken, exit." <<std::endl;
                return 1;
            }

        } catch (const std::exception& e) {

            std::cerr<< e.what() <<std::endl;

        }

    }
}

