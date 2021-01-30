//
// Created by 田地 on 2021/1/28.
//

#ifndef PROJ1_UTILS_H
#define PROJ1_UTILS_H

#include <sys/types.h>

#include <cstddef>

namespace echo {
    ssize_t WriteSocket(int sockfd, const void *buf, int len);
} // namespace echo

#endif //PROJ1_UTILS_H
