//
// Created by 田地 on 2021/3/5.
//

#include <cerrno>
#include <unistd.h>
#include "io.h"

int64_t SBCP::readlen(int fd, void* buf, int64_t size) {
    int done = 0, total = 0;

    do {

        if ((done = read(fd, buf, size)) > 0) {
            total += done;
        }

    } while ((done == -1 && errno == EINTR) || (done > 0 && total < size));

    if (done <= 0) {
        return done;
    }

    return total;
}

int64_t SBCP::writelen(int fd, void* buf, int64_t size) {
    int done = 0, total = 0;

    do {

        if ((done = write(fd, buf, size)) > 0) {
            total += done;
        }

    } while ((done == -1 && errno == EINTR) || (total < size));

    if (done <= 0) {
        return done;
    }

    return total;
}