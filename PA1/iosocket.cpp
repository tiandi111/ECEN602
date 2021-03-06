//
// Created by 田地 on 2021/1/28.
//

#include "iosocket.h"

#include <unistd.h>
#include <sys/socket.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>

#define MAX_RETRY   3

// isPrefix: If the returned line is complete, isPrefix is set to false. Otherwise, it is set to true.
// dst: The buffer to store the line
// max: The size of the buffer
// Return value: The actual bytes read and put in the buffer (include '\0')
ssize_t echo::IOSocket::ReadLine(bool* isPrefix, void* dst, ssize_t max) {
    ssize_t readl;
    ssize_t totalCopied = 0, maxLen = max;
    void* dstCur = dst;
    int retry = 0;

    do {
        while ((readl = recv(sockfd, buf + cur, std::min(BUFFER_SIZE - cur, std::max((ssize_t)1, maxLen-1)), 0)) > 0) {
            // std::cout << "readl = " << readl << std::endl;

            retry = 0;
            ssize_t copied = 0, lineEnd = cur, readEnd = cur + readl;

            // Stops when the current line hits the end of the read content / the destination buffer is full / a complete line is readed
            while ((lineEnd < readEnd) && (copied < maxLen-1) && !isTerminated(lineEnd)) {
                lineEnd++;
                copied++;
            }

            // std::cout << "readEnd = " << readEnd << ", cur = " << cur << ", copied = " << copied << ", lineEnd = " << lineEnd << std::endl;

            memcpy(dstCur, buf + cur, lineEnd - cur);
            totalCopied += copied;
            cur = lineEnd == BUFFER_SIZE ? 0 : lineEnd;

            // std::cout << "totalCopied = " << totalCopied << std::endl;
            // std::cout << "cur = " << cur << ", maxLen = " << maxLen << ", dstCur = " << dstCur << std::endl;


            // When lineEnd == readEnd, buf[readEnd] must not be a terminated charactor and there is at least one element space
            // left for '\0' in the buffer, continue the loop to read more data
            if (lineEnd != readEnd) {
                if (isTerminated(lineEnd)) {
                    // std::cout << "isTerminated(lineEnd): copied = " << copied << ", return " << totalCopied + 1 << std::endl;
                    if (isPrefix) { *isPrefix = false; }
                    ((char *) dstCur)[copied] = '\0';
                    return totalCopied + 1;
                }

                // The read data size exceeds the size of the destination buffer
                if (copied == maxLen-1) {
                    // std::cout << "copied == maxLen-1: maxLen-1 = " << maxLen-1 << ", return " << totalCopied + 1 << std::endl;
                    if (isPrefix) { *isPrefix = true; }
                    ((char*)dstCur)[maxLen-1] = '\0';
                    return totalCopied+1;
                }

                assert(0); // Should nerver reach here
            }

            dstCur = (char *) dst + copied;
            maxLen -= copied;

            // std::cout << "maxLen = " << maxLen << ", dstCur = " << dstCur << std::endl;
        }
    } while (readl == -1 && errno == EINTR && retry++ < MAX_RETRY);

    if(readl < 0) return readl;

    if(readl == 0) closed = true;

    if (isPrefix) { *isPrefix = false; }
    // std::cout << "Return " << totalCopied << std::endl;
    return totalCopied;
}

/*
     * WriteSocket writes len characters from buf into the socket.
     * will retry until all len characters has been written.
     * @param sockfd    target socket.
     * @param buf       char buffer.
     * @param len       length to write.
     * @return upon successful write, the length of characters is returned; otherwise, return -1.
     */
ssize_t echo::IOSocket::Write(int sockfd, const void *buf, int len) {
    ssize_t sent;
    ssize_t total = len;
    int retry = 0;

    do {
        while( (sent = send(sockfd, buf, len, 0)) >= 0 && (len -= sent) > 0) {
            retry = 0;
            buf = (char *) buf + sent;
        }

    } while (sent == -1 && errno == EINTR && retry++ < MAX_RETRY);

    if (sent < 0) return sent;

    return total;
}

int echo::IOSocket::CloseSocket(int sockfd) {
    int err;
    do {

        err = close(sockfd);

    } while (err == -1 && errno ==EINTR);

    return err;
}