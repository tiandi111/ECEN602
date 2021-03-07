//
// Created by Junjie Wang on 2021-03-06.
//

#ifndef ECEN602_SBCPSERVER_H
#define ECEN602_SBCPSERVER_H

#include <sys/socket.h>

#include <unordered_set>

#include "protocol.h"

#define CONN_BUF_SIZE   1024

namespace SBCP {

    class SBCPConn {

    public:
        int sockfd;
        uint16_t timeout;
        char *rbuf;
        char *wbuf;
        uint16_t bufSize;
        bool closed;
        std::string username;

        SBCPConn(int _sockfd, uint16_t _timeout, uint16_t _bufSize);
        ~SBCPConn();

        // Return the message if sccuess. If the message is not complete, it throws an exception.
        Message ReadSBCPMsg();

        // Write msg to the connection. It throws an exception when errors happen
        void WriteSBCPMsg(Message msg);

        // To check if the connection is closed by the client
        bool isClosed();
    };


    class SBCPServer {

        int listenSockFD;
        uint16_t port;
        std::string addr;
        int maxclients;

        std::unordered_set<SBCPConn> conns;
        fd_set readfds;

    public:
        SBCPServer(const std::string &_addr, uint16_t port, int _maxclients);
        void Start();

        // Get all user names
        std::vector<std::string> GetUsernames();

        // Broadcast msg to all users except the one with the given name
        void Broadcast(std::string username, Message msg);
    };


}

#endif //ECEN602_SBCPSERVER_H
