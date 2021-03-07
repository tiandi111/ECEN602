//
// Created by Junjie Wang on 2021-03-06.
//

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include "sbcpserver.h"

namespace SBCP {

    SBCPConn::SBCPConn(int _sockfd, uint16_t _timeout, uint16_t _bufSize) : sockfd(_sockfd), timeout(_timeout), bufSize(_bufSize) {
        rbuf = new char[_bufSize];
        wbuf = new char[_bufSize];
    }

    SBCPConn::~SBCPConn() {
        delete[] rbuf;
        delete[] wbuf;
    }

    // Return the message if sccuess. If the message is not complete, it throws an exception.
    Message SBCPConn::ReadSBCPMsg() {
        if (closed) return Message();

        int lenRead = read(sockfd, rbuf, bufSize);
        Message msg;

        if (lenRead == 0) {
            closed = true;
            return msg;
        }

        if (lenRead < MSG_HEADER_SIZE) {
            throw std::runtime_error("Incomplete SBCP message: " + std::string((char *)rbuf, lenRead));
        }

        msg.ReadHeader(rbuf);

        if (MSG_HEADER_SIZE + msg.GetLen() < lenRead) {
            throw std::runtime_error("Incomplete SBCP message: " + std::string((char *)rbuf, lenRead));
        }

        msg.ReadPayload((char *)rbuf + MSG_HEADER_SIZE);

        return msg;
    }

    // Write msg to the connection. It throws an exception when errors happen
    void SBCPConn::WriteSBCPMsg(Message msg) {
        if (closed) return ;

        if (MSG_HEADER_SIZE + msg.GetLen() > bufSize) {
            throw std::runtime_error("SBCP message too long! Max length: " + std::to_string(bufSize));
        }

        msg.WriteBytes(wbuf);

        int len = write(sockfd, wbuf, bufSize);
        if (len < MSG_HEADER_SIZE + msg.GetLen()) {
            throw std::runtime_error("Write incomplete SBCP message");
        }
    }

    // To check if the connection is closed by the client
    bool SBCPConn::isClosed() {
        return closed;
    }

    SBCPServer::SBCPServer(const std::string &_addr, uint16_t _port, int _maxclients) : port(_port), addr(_addr), maxclients(_maxclients) {}

    void SBCPServer::Start() {
        // Ceate socket for listen
        listenSockFD = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
        if (listenSockFD < 0) {
            throw std::runtime_error("create socket failed");
        }

        sockaddr_in sockAddr;
        bzero(&sockAddr, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = htons(port);

        if (inet_pton(AF_INET, addr.c_str(), &sockAddr.sin_addr) <= 0) {
            throw std::runtime_error("wrong IP address: " + addr);
        }

        // Bind
        if (bind(listenSockFD, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
            throw std::runtime_error("bind socket failed");
        }

        // Listen
        if(listen(listenSockFD, maxclients) < 0) {
            throw std::runtime_error(std::string("listen socket failed: ") + std::strerror(errno));
        }

        std::cout << "Server started. Listening on " << addr << ":" << port << " ..." << std::endl;

        // Handle zombie child processes
        //signal(SIGCHLD, ChildProcessHandler);

        // Start serving clients
        while (true) {

            FD_ZERO(&readfds);
            FD_SET(listenSockFD, &readfds);
            for(auto &x : conns) {
                FD_SET(x.sockfd, &readfds);
            }

            int ret = select(conns.size()+2, &readfds, nullptr, nullptr, nullptr);

            if (ret < 0) {
                throw std::runtime_error(std::string("select: ") + std::strerror(errno));
            } else if (ret == 0) {
                std::cout << "select timeout." << std::endl;
                continue;
            }

            std::vector<std::unordered_set<SBCPConn>::iterator> closedConns;

            // Handle existing connections first
            for (auto it = conns.begin(); it != conns.end(); it++) {
                SBCPConn &conn = *it;
                if (FD_ISSET(conn.sockfd, &readfds)) {
                    try {
                        Message msg = conn.ReadSBCPMsg();

                        if (conn.isClosed()) {  // Notice other users
                            closedConns.push_back(it);
                            Broadcast(conn.username, NewOfflineMessage(conn.username));  // Broadcase ONLINE message to all other users
                            continue;
                        }

                        switch(msg.GetType()) {
                            case Message::JOIN:
                                if(conns.size() >= maxclients) {
                                    conn.WriteSBCPMsg(NewNAKMessage("Full chatroom"))   // NAK
                                } else {
                                    conn.username = msg.GetAttrList()[0].GetPayloadString();    // Record username
                                    conn.WriteSBCPMsg(NewACKMessage(this->GetUsernames()));      // ACK
                                    Broadcast(conn.username, NewOnlineMessage(conn.username));  // Broadcase ONLINE message to all other users
                                }
                                break;
                            case Message::SEND:
                                Broadcast(conn.username, NewFWDMessage(conn.username, msg.GetAttrList()[0].GetPayloadString()));    // Broadcase FWD message to all other users
                                break;
                        }

                    } catch (const std::exception& e) {
                        std::cerr << e.what() << std::endl;
                        continue;
                    }
                }
            }

            // Release resources of offline users
            for (auto x : closedConns) {
                conns.erase(x);
            }

            // Accept new connections
            if (FD_ISSET(listenSockFD, &readfds)) {
                int sockAddrLen = sizeof(sockAddr);
                int newSockfd = accept(listenSockFD, (sockaddr *) &sockAddr, (socklen_t *) &sockAddrLen);
                if (newSockfd < 0) {
                    throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
                }
                std::cout << "New connection established." << std::endl;

                // Record in conns
                conns.insert(SBCPConn(newSockfd, 1000, CONN_BUF_SIZE));
            }









//
//            // Record new client in
//            sockFDs.push_back(newSockfd);
//
//
//            if (fork() == 0) {
//                ssize_t recv;
//
//                char buf[1024];
//
//                while (true) {
//                    if ((recv = read(newSockfd, buf, 1024)) > 0) {
//                        printf("recv: %s(%zd)\n", std::string(buf, recv).c_str(), recv);
//
//                    } else if (recv == 0) { // upon socket close, child process exit
//                        std::cout << "Child process exits" << std::endl;
//                        exit(0);
//                    } else {
//                        std::cerr << "read failed: errno = " << errno << std::endl;
//                    }
//                }
//            }
        }

    }

    // Get all user names
    std::vector<std::string> SBCPServer::GetUsernames() {
        std::vector<std::string> ret;

        for (auto &conn : conns) {
            ret.push_back(conn.username);
        }

        return ret;
    }

    // Broadcast msg to all users except the one with the given name
    void SBCPServer::Broadcast(std::string username, Message msg) {
        for (auto &conn : conns) {
            if (conn.username != username)
                conn.WriteSBCPMsg(msg);
        }
    }
}