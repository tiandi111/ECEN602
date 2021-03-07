//
// Created by Junjie Wang on 2021-03-06.
//

#ifndef ECEN602_SBCPSERVER_H
#define ECEN602_SBCPSERVER_H

#include <sys/socket.h>

#include <set>

#include "protocol.h"

namespace SBCP {

    class SBCPConn {
        int sockfd;
        uint16_t timeout;
        char *rbuf;
        char *wbuf;
        uint16_t bufSize;
        bool closed;

    public:
        SBCPConn(int _sockfd, uint16_t _timeout, uint16_t _bufSize);
        ~SBCPConn();

        // Return the message if sccuess. If the message is not complete, it throws an exception.
        Message ReadSBCPMsg();

        void WriteSBCPMsg(Message msg);

        bool isClosed();
    };


    class SBCPServer {

        int listenSockFD;
        uint16_t port;
        std::string addr;
        int maxclients;

        std::set<SBCPConn> conns;
        fd_set readfds;

    public:
        SBCPServer(const std::string &_addr, uint16_t port, int _maxclients);
        void Start();
    };


}

#endif //ECEN602_SBCPSERVER_H
