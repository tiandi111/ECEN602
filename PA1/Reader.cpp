//
// Created by 田地 on 2021/1/28.
//

#include "Reader.h"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

ssize_t echo::SocketReader::ReadLine(bool* isPrefix, void* dst, ssize_t max) {
    ssize_t bufEnd = cur, copied = 0, maxLen = max - 1;
    void *dstCur = dst;

    do {
        // read util a newline is met or max-1 is reached
        while ((lastRead = read(sockfd, buf + cur, BUFFER_SIZE - cur)) > 0) {
            if (lastRead == 0) { return 0; }
            if (lastRead > 0) {

                while ((bufEnd < cur + lastRead) && (copied < maxLen) && !isTerminated(bufEnd)) {
                    bufEnd++;
                    copied++;
                }

                memcpy(dstCur, buf + cur, bufEnd - cur);
                cur = bufEnd == BUFFER_SIZE ? 0 : bufEnd;

                if (isTerminated(bufEnd)) {
                    if (isPrefix) { *isPrefix = false; }
                    ((char *) dst)[copied] = '\0'; // null terminate
                    return copied + 1;
                }
                if (copied == maxLen) {
                    if (isPrefix) { *isPrefix = true; }
                    ((char *) dst)[copied] = '\0'; // null terminate
                    return copied + 1;
                }

                dstCur = (char *) dst + copied;
            }
        }
    } while (lastRead == -1 && errno == EINTR); // retry on EINTR

    return lastRead;
}