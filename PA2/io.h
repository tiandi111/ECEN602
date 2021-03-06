//
// Created by 田地 on 2021/3/4.
//

#ifndef PA2_IO_H
#define PA2_IO_H

#include <cerrno>
#include <cstdint>
#include <unistd.h>

namespace SBCP {

/**
 * read `len` number of bytes from a file descriptor.
 * this function succeeds only if all `len` number of bytes has been read.
 * @param fd the file descriptor to read from.
 * @param buf the buffer to write data.
 * @param len the number of bytes to read.
 *
 * @return return `len` on success, return -1 if error occurred,
 *      return 0 if eof occurred.
 */
int64_t readlen(int fd, void* buf, int64_t len);

/**
 * write `len` number of bytes to a file descriptor.
 * this function succeeds only if all `len` number of bytes has been written.
 * @param fd the file descriptor to write.
 * @param buf the buffer to write data.
 * @param len the number of bytes to write.
 *
 * @return return `len` on success, return -1 if error occurred,
 *      return 0 if eof occurred.
 */
int64_t writelen(int fd, void* buf, int64_t len);

}

#endif //PA2_IO_H
