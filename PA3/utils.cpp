//
// Created by 田地 on 2021/3/23.
//

#include <iostream>
#include <stdexcept>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "utils.h"

int CmpSockaddr(struct sockaddr* x, struct sockaddr* y) {
    if (x->sa_family != y->sa_family) {
        return x->sa_family - y->sa_family;
    }

    if (x->sa_family != AF_INET) {
        throw std::runtime_error("unsupported sa_family: " + std::to_string(x->sa_family));
    }

#define IN_ADDR(sa) ((sockaddr_in*)(sa))->sin_addr.s_addr
#define IN_PORT(sa) ((sockaddr_in*)(sa))->sin_port

    if (IN_ADDR(x) != IN_ADDR(y)) {
        return IN_ADDR(x) - IN_ADDR(y);
    }

    if (IN_PORT(x) != IN_PORT(y)) {
        return IN_PORT(x) - IN_PORT(y);
    }

    return 0;
}