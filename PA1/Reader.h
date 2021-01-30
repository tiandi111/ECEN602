//
// Created by 田地 on 2021/1/28.
//

#ifndef PROJ1_READER_H
#define PROJ1_READER_H

#include <sys/types.h>

#include <cstdio>

#define BUFFER_SIZE 2048


namespace echo {

/*
 * SocketReader a helper class to read character from a given socket.
 */
class SocketReader {
  public:
    SocketReader(int _sockfd) : sockfd(_sockfd), cur(0) {}

    ~SocketReader() = default;

    /*
     * ReadLine reads a line into the given buffer.
     * If no newline character received, read max-1 character with null terminator appended.
     * @param isPrefix  if true, indicates that no newline character exists, can be nullptr.
     * @param buf       char buffer to read  .
     * @param max       the maximum number of characters to read.
     * @return the length of the line, if error occurred return -1.
     */
    ssize_t ReadLine(bool* isPrefix, void* buf, ssize_t max);

    /*
     * Reset redirects the reader to a new socket.
     * will clean all static states.
     * @param _sockdf   new socket to read.
     */
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
