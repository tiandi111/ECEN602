//
// Created by 田地 on 2021/4/2.
//

#include <string>
#include <memory>

#ifndef PA3_IO_H
#define PA3_IO_H

namespace TFTP {

namespace IO {

class IFileReader;
using UniqueIFileReader = std::unique_ptr<IFileReader>;

class NetasciiFileReader;
using UniqueNetasciiFileReader = std::unique_ptr<NetasciiFileReader>;

class OctetFileReader;
using UniqueOctetFileReader = std::unique_ptr<OctetFileReader>;

class DummyIFileReader;
using UniqueDummyIFileReader = std::unique_ptr<DummyIFileReader>;

/**
 * File Reader Interface
 */
class IFileReader {
  public:
    /**
     * Read bytes from file into buffer
     * @param buf buffer to write data
     * @param size number of bytes to read
     * @return number of bytes on success(0 indicates EOF),
     *      -1 if error occurred.
     */
    virtual int Read(char* buf, int size) = 0 ;

    /**
     * Destructor
     */
    virtual ~IFileReader() = default ;

};

/**
 * File reader factory
 * @param type file reader type, can be "netascii", "octet" or "test"
 * @param filepath
 * @return unique_ptr of the created file reader, null if type is invalid
 */
UniqueIFileReader UniqueIFileReaderFactory(std::string type, std::string filepath) ;

/**
 * File reader that reads data and convert to "netascii" format
 */
class NetasciiFileReader : public IFileReader {
  public:
    /**
     * Parameterized constructor
     * @param filepath
     * @throws std::runtime_error thrown if failed to open file
     */
    NetasciiFileReader(std::string filepath) ;

    /**
     * Create unique_ptr of NetasciiFileReader
     * @param filepath
     * @return unique_ptr of created NetasciiFileReader
     * @throws std::runtime_error thrown if failed to open file
     */
    static UniqueNetasciiFileReader Create(std::string filepath) ;

    /**
     * Destructor
     */
    ~NetasciiFileReader() ;

    /**
     * Override Read function
     * @see Read of IFileReader
     */
    int Read(char* buf, int size) override ;

  private:
    FILE* file;
    int next;
};

/**
 * File reader that reads data and convert to "octet" format
 */
class OctetFileReader : public IFileReader {
  public:
    /**
     * Parameterized constructor
     * @param filepath
     * @throws std::runtime_error thrown if failed to open file
     */
    OctetFileReader(std::string filepath) ;

    /**
     * Create unique_ptr of OctetFileReader
     * @param filepath
     * @return unique_ptr of created OctetFileReader
     * @throws std::runtime_error thrown if failed to open file
     */
    static UniqueOctetFileReader Create(std::string filepath) ;

    /**
     * Destructor
     */
    ~OctetFileReader() ;

    /**
     * Override Read function
     * @see Read of IFileReader
     */
    int Read(char* buf, int size) override ;

  private:
    int fd;
};

/*
 * File reader that is used for testing only
 */
class DummyIFileReader : public TFTP::IO::IFileReader {
  public:
    DummyIFileReader(std::string filepath) ;

    static UniqueDummyIFileReader Create(std::string filepath) ;

    ~DummyIFileReader() = default;

    // always return size
    int Read(char* buf, int size) override ;
};

} // namespace IO

} // namespace TFTP

#endif //PA3_IO_H
