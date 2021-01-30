//
// Created by 田地 on 2021/1/28.
//

#ifndef PROJ1_UTILS_H
#define PROJ1_UTILS_H

#include <sys/types.h>

#include <cstddef>

namespace echo {
    /*
     * WriteSocket writes len characters from buf into the socket.
     * will retry until all len characters has been written.
     * @param sockfd    target socket.
     * @param buf       char buffer.
     * @param len       length to write.
     * @return the length of characters written, -1 if error occurred.
     */
    ssize_t WriteSocket(int sockfd, const void *buf, int len);
} // namespace echo

#endif //PROJ1_UTILS_H
