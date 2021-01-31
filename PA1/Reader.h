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
 * Notice that SocketReader is only responsible for reading, users should manage the socket lifecycle.
 * NOT Concurrently Safe!
 */
class SocketReader {
  public:
    SocketReader(int _sockfd) : sockfd(_sockfd), cur(0) {}

    ~SocketReader() = default;

    /*
     * ReadLine reads a line from a socket into a buffer.
     * If no newline character received, read max-1 character with null terminator appended.
     * @param isPrefix  if true, indicates that no newline character exists, can be nullptr.
     * @param buf       char buffer to read  .
     * @param max       the maximum number of characters to read.
     * @return upon successful read, the length of the line is returned; if error occurred returns -1;
     *      if the socket is closed, 0 is returned;
     */
    ssize_t ReadLine(bool* isPrefix, void* buf, ssize_t max);

    /*
     * Closed checks if the socket has been closed.
     * @return true if closed; false if not.
     */
    bool SocketClosed() {
        return lastRead == 0;
    }

    /*
     * Reset redirects the reader to a new socket and clean all static states.
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
    ssize_t lastRead; // store the last return value of read
    int sockfd;
};

} // namespace echo



#endif //PROJ1_READER_H
