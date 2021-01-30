//
// Created by 田地 on 2021/1/28.
//

#ifndef PROJ1_READER_H
#define PROJ1_READER_H

#include <sys/types.h>

#include <cstdio>

#define BUFFER_SIZE 2048


namespace echo {

class SocketReader {
  public:
    SocketReader(int _sockfd) : sockfd(_sockfd), cur(0) {}

    ~SocketReader() = default;

    ssize_t ReadLine(bool* isPrefix, void* buf, ssize_t max);

    inline void Reset(int _sockfd) {
        sockfd = _sockfd;
        cur = 0;
    }

  private:
    inline bool isTerminated(ssize_t idx) {
        return buf[idx] == '\n' || buf[idx] == '\0' || buf[idx] == EOF;
    }

    char buf[BUFFER_SIZE];
    ssize_t cur;
    int sockfd;
};

} // namespace echo



#endif //PROJ1_READER_H
