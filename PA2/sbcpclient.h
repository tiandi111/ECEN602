//
// Created by 田地 on 2021/2/28.
//

#ifndef PROJ1_SBCPCLIENT_H
#define PROJ1_SBCPCLIENT_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "protocol.h"

namespace SBCP {

/**
 * Client that implements SBCP protocol.
 */
class SBCPClient {
  #define CLT_BUFFER_SIZE 1024

  public:
    SBCPClient() = default;

    /**
     * construct a client.
     * @param username client username.
     * @param addr server address.
     * @param port server port.
     */
    SBCPClient(std::string username, std::string addr, uint16_t port);

    /**
     * default destructor
     */
    ~SBCPClient();

    /**
     * initialize client resources and join the chat server.
     * @return true if entering the chat room successfully. Otherwise, return false.
     */
    bool Init();

    /**
     * start client.
     * the client will read message lines from standard input, send messages
     * to server and display all messages received from server.
     */
    int Start();

    /**
     * block util some events happens.
     * @return number of events happened.
     *
     * @throw std::runtime_error thrown if wait events failed.
     *
     * @note after this function returns, should call event handle functions,
     *      @see `ForwardMessages`, `RecvMessages` and etc..
     */
    int WaitEvent();

    /**
     * forward a line of characters from standard input if any.
     *
     * @throw std::runtime_error thrown if forward messages failed.
     *
     * @return true if fgets gets no input. Otherwise, return false.
     *
     * @note should call this function after `WaitEvent` returns.
     */
    bool ForwardMessages();

    /**
     * receive and display a message from server if any.
     *
     * @throw std::runtime_error thrown if receive messages failed.
     *
     * @note should call this function after `WaitEvent` returns.
     */
    void RecvMessages();

    /**
     * send data to server.
     * this function makes sure that all `len` number of bytes will be
     * sent, otherwise, an exception will be thrown.
     *
     * @param src pointer to the message to be sent.
     * @param len number of bytes to send.
     *
     * @throw std::runtime_error thrown if send data failed.
     *
     */
    void Send(void* src, size_t len);

    /**
     * return if connection is closed.
     * @return closed.
     *
     * @note should call this function after `RecvMessages`
     */
    bool IsClosed();

  private:
    std::string username;
    in_port_t port;
    std::string addr;
    int sockfd;
    sockaddr_in sockAddr;
    fd_set readfds;
    bool closed;
};

}


#endif //PROJ1_SBCPCLIENT_H
