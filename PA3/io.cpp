//
// Created by 田地 on 2021/4/2.
//

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "io.h"

TFTP::IO::UniqueIFileReader TFTP::IO::UniqueIFileReaderFactory(std::string type, std::string filepath) {
    if (type == "netascii") {
        return TFTP::IO::NetasciiFileReader::Create(filepath);
    } else if (type == "octet") {
        return TFTP::IO::OctetFileReader::Create(filepath);
    } else if (type == "test") {
        return TFTP::IO::DummyIFileReader::Create(filepath);
    } else {
        return nullptr;
    }
}

/// ===========================================
///               NetasciiFileReader
/// ===========================================
TFTP::IO::UniqueNetasciiFileReader TFTP::IO::NetasciiFileReader::Create(std::string filepath) {
    return TFTP::IO::UniqueNetasciiFileReader (new TFTP::IO::NetasciiFileReader(filepath) );
}

TFTP::IO::NetasciiFileReader::NetasciiFileReader(std::string filepath) : next(-1) {
    file = fopen(filepath.c_str(), "r");
    if ( !file ) {
        throw std::runtime_error(std::string("fopen: ") + strerror(errno));
    }
}

TFTP::IO::NetasciiFileReader::~NetasciiFileReader() {
    if ( file ) fclose(file);
}

int TFTP::IO::NetasciiFileReader::Read(char* buf, int size) {
    for (int i = 0; i < size; i++ ) {
        if (next >= 0) {
            buf[i] = next;
            next = -1;
            continue;
        }
        int c = fgetc(file);
        switch (c) {
            case EOF:
                if ( feof(file) ) return i;
                if ( ferror(file) ) return -1;
            case 10: // LF -> CR LF
                buf[i] = 13;
                next = 10;
                break;
            case 13: // CR -> CR NUL
                buf[i] = 13;
                next = 0;
                break;
            default:
                buf[i] = c;
        }
    }
    return size;
}

/// ===========================================
///               OctetFileReader
/// ===========================================
TFTP::IO::UniqueOctetFileReader TFTP::IO::OctetFileReader::Create(std::string filepath) {
    return UniqueOctetFileReader (new OctetFileReader(std::move(filepath)) );
}

TFTP::IO::OctetFileReader::OctetFileReader(std::string filepath) {
    fd = open(filepath.c_str(), O_RDONLY);
    if ( fd <= 0 ) {
        throw std::runtime_error(std::string("open: ") + strerror(errno));
    }
}

TFTP::IO::OctetFileReader::~OctetFileReader() {
    if ( fd > 0 ) {
        close(fd);
    }
}

int TFTP::IO::OctetFileReader::Read(char *buf, int size) {
    return read(fd, buf, size);
}

/// ===========================================
///               DummyIFileReader
/// ===========================================
TFTP::IO::DummyIFileReader::DummyIFileReader(std::string filepath) {}

TFTP::IO::UniqueDummyIFileReader TFTP::IO::DummyIFileReader::Create(std::string filepath) {
    return UniqueDummyIFileReader (new DummyIFileReader(filepath));
}

int TFTP::IO::DummyIFileReader::Read(char* buf, int size) {
    return size;
}