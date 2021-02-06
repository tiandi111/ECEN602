//
// Created by 田地 on 2021/1/28.
//

#ifndef PROJ1_IOSOCKET_H
#define PROJ1_IOSOCKET_H

#include <sys/types.h>
#include <iostream>

#include <cstdio>

#define BUFFER_SIZE 2048


namespace echo {

class IOSocket {
  public:
    IOSocket(int _sockfd) : sockfd(_sockfd), cur(0), closed(false) {}

    ~IOSocket() = default;

    ssize_t ReadLine(bool* isPrefix, void* buf, ssize_t max);
    ssize_t Write(int sockfd, const void *buf, int len);

    /*
     * Closed checks if the socket has been closed.
     * @return true if closed; false if not.
     */
    inline bool SocketClosed() {
        return closed;
    }

    /*
     * Reset redirects the reader to a new socket and clean all static states.
     * @param _sockdf   new socket to read.
     */
    inline void Reset(int _sockfd) {
        sockfd = _sockfd;
        cur = 0;
        closed = false;
    }

  private:
    inline bool isTerminated(ssize_t idx) {
        return buf[idx] == '\n' || buf[idx] == '\0' || buf[idx] == EOF;
    }

    char buf[BUFFER_SIZE];
    ssize_t cur;
    int sockfd;
    bool closed;
};

} // namespace echo


#endif //PROJ1_IOSOCKET_H
