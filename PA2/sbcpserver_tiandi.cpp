//
// Created by 田地 on 2021/2/28.
//

#include <string>
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include "io.h"
#include "sbcpserver_tiandi.h"
#include "protocol.h"

SBCP::Client::Client(int _socket) : socket(_socket), state(INIT) {}

void SBCP::Client::Join(const std::string& _username) {
    state = JOINED;
    username = _username;
}

void SBCP::Client::Offline() {
    state = OFFLINE;
}

void SBCP::Client::Invalid() {
    state = INVALID;
}

bool SBCP::Client::IsReady() {
    return state > INIT && state < OFFLINE;
}

bool SBCP::Client::IsOffline() {
    return state == OFFLINE;
}

bool SBCP::Client::IsInvalid() {
    return state == INVALID;
}

SBCP::SBCPServer::SBCPServer(uint16_t _port, const std::string &_addr, int _backlog) :
    port(_port), addr(_addr), backlog(_backlog) {}

void SBCP::SBCPServer::Init() {
    sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if (sockfd < 0) {
        throw std::runtime_error(std::string("socket: ") + std::strerror(errno));
    }

    bzero(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, addr.c_str(), &sockAddr.sin_addr) <= 0) {
        throw std::runtime_error("wrong IP address: " + addr);
    }

    if (bind(sockfd, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error(std::string("bind: ") + std::strerror(errno));
    }

    if (listen(sockfd, backlog) < 0) {
        throw std::runtime_error(std::string("listen: ") + std::strerror(errno));
    }

    maxFD = sockfd;
}

void SBCP::SBCPServer::AcceptClient() {
    if (FD_ISSET(sockfd, &readfds)) {
        int sockAddrLen = sizeof(sockAddr);
        int newSockfd = accept(sockfd, (sockaddr *) &sockAddr, (socklen_t *) &sockAddrLen);
        if (newSockfd <= 0) {
            throw std::runtime_error(std::string("accept: ") + std::strerror(errno));
        } else {
            if (clients.size() >= MAX_CLT_NUM) {
                close(newSockfd);
            } else {
                clients.emplace_back(Client(newSockfd));
            }
        }
    }
}

void SBCP::SBCPServer::Broadcast(Message msg, int src) {
    char buf[msg.Size()];
    msg.WriteBytes(buf);
    for (auto& client : clients) {
        if (!client.IsReady() || client.socket == src) {
            continue;
        }
        if((SBCP::writelen(client.socket, buf, msg.Size()) <= 0)) {
            throw std::runtime_error(std::string("writelen: ") + std::strerror(errno));
        }
    }
}


void SBCP::SBCPServer::HandleClients() {
    for (auto it = clients.begin(); it != clients.end(); it++) {
        Client& clt = *it;
        if (FD_ISSET(clt.socket, &readfds)) {
            Message msg;
            int code = SBCP::ReadMessage(msg, clt.socket);
            if (code == 0) {
                if (clt.state > Client::INIT) {
                    clt.Offline();
                }
                continue;
            }
            if (code < 0 ) {
                throw std::runtime_error(std::string("ReadMessage: " + std::to_string(msg.GetType())));
            }
            switch (msg.GetType()) {
                case Message::JOIN: {
                    Attribute username = msg.GetAttrMap().at(Attribute::Username);
                    if (usernameSet.end() != usernameSet.find(username.GetPayloadString())) {
                        clt.Invalid();
                        continue;
                    }
                    usernameSet.emplace(username.GetPayloadString());
                    clt.Join(username.GetPayloadString());
                    Broadcast(NewFWDMessage(clt.username, "entered the chat room.\n"), clt.socket);
                    break;
                }
                case Message::SEND: {
                    Attribute data = msg.GetAttrMap().at(Attribute::Message);
                    Broadcast(NewFWDMessage(clt.username, data.GetPayloadString()), clt.socket);
                    std::cout<< clt.username << ": " << data.GetPayloadString() <<std::endl;
                    break;
                }
                default:
                    throw std::runtime_error(std::string("invalid message type: " + std::to_string(msg.GetType())));
            }
        }
    }
    for (auto it = clients.begin(); it != clients.end(); ) {
        Client& clt = *it;
        if (clt.IsOffline() || clt.IsInvalid()) {
            clients.erase(it);
            usernameSet.erase(clt.username);
            if (clt.IsOffline()) {
                Broadcast(NewFWDMessage(clt.username, "exited the chat room.\n"), clt.socket);
            }
            close(clt.socket);
        } else {
            it++;
        }
    }
}

void SBCP::SBCPServer::WaitEvent() {
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    maxFD = sockfd;
    for (auto& client : clients) {
        FD_SET(client.socket, &readfds);
        maxFD = std::max(maxFD, client.socket);
    }
    int ret = select(maxFD + 1, &readfds , NULL , NULL , NULL);
    if (ret < 0) {
        throw std::runtime_error(std::string("select: ") + std::strerror(errno));
    }
}

void SBCP::SBCPServer::Start() {
    Init();

    while (true) {

        try {
            WaitEvent();

            AcceptClient();

            HandleClients();

        } catch (const std::exception& e) {

            std::cerr<< e.what() << std::endl;

        }

    }
}

