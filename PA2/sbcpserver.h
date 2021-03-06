//
// Created by 田地 on 2021/2/28.
//

#ifndef PROJ1_SBCPSERVER_H
#define PROJ1_SBCPSERVER_H

#include <stdexcept>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "protocol.h"

namespace SBCP {

/**
 * Server-side representation of a SBCP client.
 */
class Client {
  public:
    /// Client state enum
    enum State {
        INIT,
        JOINED,
        OFFLINE,
        INVALID,
    };

    /**
     * construct a new client in `INIT` state.
     * @param socket the socket of a client connection.
     */
    Client(int socket);

    /**
     * destructor
     */
    ~Client() = default;

    /**
     * set client state to `JOIN`.
     * @param username
     */
    void Join(const std::string& username);

    /**
     * set client state to `OFFLINE`.
     */
    void Offline();

    /**
     * set client state to `INVALID`.
     */
    void Invalid();

    /**
     * return if the client is ready to chat,
     * @return if client is ready.
     */
    bool IsReady();

    /**
     * return if client is offline.
     * @return if client is offline.
     */
    bool IsOffline();

    /**
     * return if client is invalid.
     * @return if client is invalid.
     */
    bool IsInvalid();

    int socket;
    State state;
    std::string username;
};

/**
 * Server that implements SBCP protocol.
 *
 *
 */
class SBCPServer {
#define MAX_CLT_NUM 3

  public:
    /**
     * construct a server.
     * @param port the port to listen.
     * @param addr the server address.
     * @param backlog the number of backlog connection.
     */
    SBCPServer(uint16_t port, const std::string& addr, int backlog);

    /**
     * default destructor.
     */
    ~SBCPServer() = default;

    /**
     * initialize resources that is necessary for server to serve clients.
     */
    void Init();

    /**
     * start server to server clients.
     *
     */
    void Start();

    /**
     * block util some events happens.
     * @return number of events happened.
     *
     * @throw std::runtime_error thrown if wait events failed.
     *
     * @note after this function returns, should call event handle functions,
     *      @see `AcceptClient`, `HandleClients` and etc..
     */
    void WaitEvent();

    /**
     * accept a new client connection if any.
     *
     * @throw std::runtime_error thrown if accept failed.
     *
     * @note a newly accepted client is not ready for chatting,
     *      it is ready until the server received a `JOIN` message from it.
     * @note call this functions after `WaitEvent` returns.
     */
    void AcceptClient();

    /**
     * handle a message received from each client if any.
     *
     * @throw std::runtime_error thrown if handle client failed.
     *
     * @note call this functions after `WaitEvent` returns.
     */
    void HandleClients();

    /**
     * broadcast a message to all clients.
     * @param msg message to broadcast.
     * @param src the socket of the sender.
     */
    void Broadcast(Message msg, int src);

  private:
    in_port_t port;
    std::string addr;
    int backlog;
    int sockfd;
    sockaddr_in sockAddr;
    fd_set readfds;
    int maxFD;
    std::vector<Client> clients;
    std::unordered_set<std::string> usernameSet;
};

}



#endif //PROJ1_SBCPSERVER_H
