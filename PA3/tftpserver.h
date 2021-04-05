//
// Created by 田地 on 2021/3/11.
//

#ifndef PA3_TFTPSERVER_H
#define PA3_TFTPSERVER_H

#include <chrono>
#include <string>
#include <vector>

#include <netinet/in.h>

#include "tftpsession.h"

namespace TFTP {

using MilliSecond = int;

/**
 * server config struct
 */
struct ServerConfig {
    /// @brief server address
    std::string addr;
    /// @brief listening port
    int port;
    /// @brief request backlog
    int backlog;
    /// @brief TFTP protocol resend timeout
    std::chrono::milliseconds timeout;
};

using RequestSessions = std::vector<Session::UniqueRequestSession>;

/**
 * tftp server that implements I/O multiplexing
 */
class PollServer {
  public:
    /**
     * parameterized constructor
     * @param _serverConfig server config struct
     */
    PollServer(struct ServerConfig _serverConfig) ;

    /**
     * destructor
     */
    ~PollServer() ;

    /**
     * start server
     * @throws std::runtime_error thrown if daemon session failed to start
     */
    void Start() ;

  private:
    /// @breif server config struct
    ServerConfig serverConfig;
    /// @breif daemon session that accepts and creates new request sessions
    Session::UniqueDaemonSession daemonSession;
    /// @breif ongoing request session vector
    RequestSessions requestSessions;
};

} // namespace TFTP



#endif //PA3_TFTPSERVER_H
