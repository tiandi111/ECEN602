//
// Created by 田地 on 2021/3/14.
//

#ifndef PA3_TFTPSESSION_H
#define PA3_TFTPSESSION_H

#include <chrono>
#include <stdexcept>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <unistd.h>

#include <arpa/inet.h>

#include "io.h"

namespace TFTP {

namespace Session {

/**
 * TFTP packet opcode
 */
enum Opcode {
    ST,
    RRQ = 1,
    WRQ,
    DATA,
    ACK,
    ERROR,
    END
};

class DaemonSession;
using UniqueDaemonSession = std::unique_ptr<DaemonSession>;

class RequestSession;
using UniqueRequestSession = std::unique_ptr<RequestSession>;

class RRQSession;
using UniqueRRQSession = std::unique_ptr<RRQSession>;

using FileDescriptor = int;

using namespace std::chrono;

/**
 * session that accepts and creates request sessions
 */
class DaemonSession {
  public:
    /**
     * parameterized constructor
     * @param addr
     * @param port
     * @param backlog
     */
    DaemonSession(std::string addr, int port, int backlog);

    /**
     * destructor
     */
    ~DaemonSession();

    /**
     * starts listening for requests
     * @throws std::runtime_error thrown if start listening failed
     */
    void Init();

    /**
     * accepts and creates a new request session
     * @param timeout tftp request session resend timeout
     * @return unique_ptr of created request session
     */
    UniqueRequestSession AcceptRequest(milliseconds timeout);

    /**
     * returns socket file descriptor
     * @return socket file descriptor
     */
    const TFTP::Session::FileDescriptor Socket() const;

    /**
     * creates unique_ptr of daemon session
     * @param addr
     * @param port
     * @param backlog
     * @return unique_ptr of created daemon session
     */
    static UniqueDaemonSession Create(std::string addr, int port, int backlog);

  private:
    char buf[1024];
    std::string addr;
    int port;
    int backlog;
    sockaddr_in sockAddr;
    TFTP::Session::FileDescriptor sockfd;
};

class RequestSession {
#define MAX_RETRY 10
  public:
    /**
     * tftp error code
     */
    enum ErrorCode {
        Undefined,
        FileNotFound,
        AccessViolation,
        DiskFull,
        IllegalOperation,
        UnknownTransferID,
        FileExists,
        NoSuchUser,
    };

    /**
     * parameterized constructor
     * @param destAddr destination sockaddr
     * @param path path of the file to read or write
     * @param mode transfer mode, "netasicc", "octet" or "test" (for testing only)
     * @param timeout resend timeout
     * @throws std::runtime_error thrown if mode is invalid or create socket failed
     */
    RequestSession(sockaddr destAddr, std::string path, std::string mode, milliseconds timeout);

    RequestSession(RequestSession &&other) = delete;

    /**
     * destructor
     *
     * this method only cleanup resources managed within this class
     */
    virtual ~RequestSession() noexcept;

    /**
     * Proceeds session if possible
     *
     * Two actions can be taken, either send a data packet to client or
     * receive an ack packet from client, which action to take is indicated
     * by readReady and writeReady.
     *
     * The expected way to use this method is to first check the socket status
     * by select(), poll(), epoll() or equivalents. If socket is read-ready, that
     * is, reading from the socket will not block, then `readReady` should be set
     * to true. Same for `writeReady`.
     *
     * If socket is not read-ready but `readReady` is true, this method will block
     * until socket is read-ready. Same for `writeReady`.
     *
     * @param readReady true indicates a receive packet try
     * @param writeReady true indicates a send packet try
     * @note if Terminated() returns true, this method will return immediately
     */
    virtual void Proceeds(bool readReady, bool writeReady) = 0;

    /**
     * Terminates session
     */
    void Terminate();

    /**
     * Virtual method that Returns if session is terminated
     * @return if session is terminated
     */
    bool Terminated() ;

    /**
     * Returns file path
     * @return file path
     */
    const std::string Path() const;

    /**
    * Returns transfer mode
    * @return transfer mode, "netasicc", "octet" or "test" (for testing only)
    */
    const std::string Mode() const;

    /**
     * Returns socket file descriptor
     * @return socket file descriptor
     */
    const TFTP::Session::FileDescriptor Socket() const;

    /**
     * Returns error message of termination
     * @return error message of termination, empty if session is not terminated
     *      or terminated normally(file transferred successfully)
     */
    const std::string ErrMsg();

    /**
     * Virtual method that returns request session type string
     * @return request session type string, "RRQ" or "WRQ"
     */
    virtual const std::string TypeStr() = 0;

    /**
     * Sends an error packet to client
     * @param ec error code
     * @param msg error message
     * @throws std::runtime_error thrown if send failed
     */
    void SendErrorPacket(ErrorCode ec, std::string msg);

    /**
     * Returns request session unique ID string
     * @return request session unique ID string
     */
    const std::string ID();

  protected:
    void cleanup();

    int sendErrorPacket(ErrorCode ec, std::string msg);

    sockaddr destAddr;
    TFTP::Session::FileDescriptor sockfd;
    std::string path;
    std::string mode;
    std::string id;
    milliseconds timeout;
    std::string errmsg;
    bool closed: 1;
};

class RRQSession : public RequestSession {
  public:
    /**
     * Parameterized constructor
     * @param destAddr client sockaddr
     * @param path path of the file to read
     * @param mode file transfer mode, "netasicc", "octet" or "test"(for testing only)
     * @param timeout resend timeout
     * @return unique_ptr of created RRQSession
     * @throws std::runtime_error thrown if transfer mode unsupported
     */
    RRQSession(sockaddr destAddr, std::string path, std::string mode, milliseconds timeout);

    /**
     * Creates unique_ptr o RRQSession
     * @param destAddr client sockaddr
     * @param path path of the file to read
     * @param mode file transfer mode, "netasicc", "octet" or "test"(for testing only)
     * @param timeout resend timeout
     * @return unique_ptr of created RRQSession
     * @throws std::runtime_error thrown if transfer mode unsupported
     */
    static UniqueRRQSession Create(sockaddr destAddr, std::string path, std::string mode, milliseconds timeout);

    /**
     * Destructor
     */
    ~RRQSession() = default;

    RRQSession(RRQSession &&other) = delete;
    RRQSession(RRQSession &other) = delete;
    RRQSession &operator=(const RRQSession &) = delete;

    /**
     * Overrided Proceeds
     * @see Proceeds of RequestSession
     */
    void Proceeds(bool readReady, bool writeReady) override;

    /**
     * Overrided TypeStr method
     * @return "RRQ" string
     */
    const std::string TypeStr();

  private:
    // receives a packet
    void recvAck();
    // sends a data packet
    void sendData();
    // returns if data packet is ready to be sent
    bool dataReady();
    // returns if acknowledgement timeouts
    bool ackTimeout();

    char dataBuf[516];
    char recvBuf[512];
    int dataIn;
    int acked;
    int curblk;
    int tryNum;
    bool dupAck: 1;
    bool eof: 1;
    IO::UniqueIFileReader reader;
    steady_clock::time_point lastSentTime;
};

} // namespace Session

} // namespace TFTP


#endif //PA3_TFTPSESSION_H
