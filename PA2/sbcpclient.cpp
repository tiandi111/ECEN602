#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "io.h"
#include "sbcpclient.h"
#include "protocol.h"

SBCP::SBCPClient::SBCPClient(std::string _username, std::string _addr, uint16_t _port)
    : username(std::move(_username)),
      port(_port),
      addr(std::move(_addr)),
      closed(false)
      {}

SBCP::SBCPClient::~SBCPClient() {
    close(sockfd);
}

bool SBCP::SBCPClient::Init() {
    std::cout<< "Connecting to server..." <<std::endl;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error(std::string("socket: ") + std::strerror(errno));
    }

    bzero(&sockAddr, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, addr.c_str(), &sockAddr.sin_addr) <= 0) {
        throw std::runtime_error("wrong IP address: " + addr);
    }

    // Connect to the server
    if (connect(sockfd, (sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        throw std::runtime_error(std::string("connect: ") + std::strerror(errno));
    }

    // Send JOIN message
    std::cout<< "Joining in the chat room..." <<std::endl;
    Message joinMsg = NewJoinMessage(username);
    char buf[joinMsg.Size()];
    joinMsg.WriteBytes(buf);
    if(send(sockfd, buf, joinMsg.Size(), 0) < 0) {
        throw std::runtime_error(std::string("send: ") + std::strerror(errno));
    }

    // Get response from the server
    Message rspMsg;
    int ret = SBCP::ReadMessage(rspMsg,  sockfd);
    if (ret == 0) {
        closed = true;
        throw std::runtime_error(std::string("Connection closed by the server"));
    }
    if (ret < 0 ) {
        throw std::runtime_error(std::string("ReadMessage: ") + std::strerror(errno));
    }

    // The response is expected to be ACK or NAK
    if (rspMsg.GetType() == Message::ACK) {
        std::string userCount;

        std::cout<< "You enter the chat room." <<std::endl;

        for (auto &x : rspMsg.GetAttrList()) {
            if (x.GetType() == Attribute::ClientCount) {
                std::cout<< "Current user number: " << x.GetPayloadString() << std::endl; std::cout.flush();
            }
            else if (x.GetType() == Attribute::Username) {
                std::cout<< x.GetPayloadString() << std::endl; std::cout.flush();
            }
        }
        return true;
    } else if (rspMsg.GetType() == Message::NAK) {
        if (rspMsg.GetAttrList().size() != 1 || rspMsg.GetAttrList()[0].GetType() != Attribute::Reason)
            throw std::runtime_error(std::string("Wrong message format"));
        std::cout<< "Join rejected by the server. Reason: " << rspMsg.GetAttrList()[0].GetPayloadString() << std::endl; std::cout.flush();
        return false;
    }

    throw std::runtime_error(std::string("Unexpected response from the server"));
}

int SBCP::SBCPClient::WaitEvent() {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sockfd, &readfds);
    int ret = select(std::max(sockfd, STDIN_FILENO) + 1, &readfds , NULL , NULL , NULL);
    if (ret < 0) {
        throw std::runtime_error(std::string("select: ") + std::strerror(errno));
    } else if (ret == 0) {
        std::cout << "select timeout." << std::endl; std::cout.flush();
    }
    return ret;
}

void SBCP::SBCPClient::ForwardMessages() {
    char buf[CLT_BUFFER_SIZE];
    if (FD_ISSET(STDIN_FILENO, &readfds) &&
        fgets(buf, CLT_BUFFER_SIZE, stdin) != NULL /*&&
        strlen(buf) > 0*/) {
//        std::cout << "ForwardMessages" << std::endl; std::cout.flush();
        Send(buf, strlen(buf));
    }
}

void SBCP::SBCPClient::RecvMessages() {
    if (FD_ISSET(sockfd, &readfds)) {
//        std::cout<< "RecvMessages" << std::endl; std::cout.flush();

        Message msg;
        int code = SBCP::ReadMessage(msg,  sockfd);
        if (code == 0) {
            closed = true;
            return;
        }
        if (code < 0 ) {
            throw std::runtime_error(std::string("ReadMessage: ") + std::strerror(errno));
        }
        if (msg.GetType() == Message::FWD) {
            auto username = msg.GetAttrList()[0].GetPayloadString();
            auto info = msg.GetAttrList()[1].GetPayloadString();
            std::cout<< username << ": " << info; std::cout.flush();
        } else if (msg.GetType() == Message::ONLINE) {
            std::cout<< msg.GetAttrList()[0].GetPayloadString() << " is now online." << std::endl; std::cout.flush();
        } else if (msg.GetType() == Message::OFFLINE) {
            std::cout<< msg.GetAttrList()[0].GetPayloadString() << " is now offline." << std::endl; std::cout.flush();
        } else {
            throw std::runtime_error(std::string("Unexpected message type ") + std::to_string(msg.GetType()));
        }
    }
}

void SBCP::SBCPClient::Send(void* ptr, size_t len) {
    Message msg =  NewSendMessage(std::string((char *) ptr, len));
    char buf[msg.Size()];
    msg.WriteBytes(buf);
    if (SBCP::writelen(sockfd, buf, msg.Size()) < 0) {
        throw std::runtime_error(std::string("writelen: ") + std::strerror(errno));
    }
}

bool SBCP::SBCPClient::IsClosed() {
    return closed;
}

int SBCP::SBCPClient::Start() {
    try {
        if(!Init()) // Connect to the server and send JOIN message
            return -1;
    } catch (const std::exception& e) {
        std::cerr<< e.what() <<std::endl; std::cerr.flush();
        return -1;
    }


    while (true) {

        std::cout<< "> "; std::cout.flush();

        try {
            int i = WaitEvent();
            std::cout << std::endl; std::cout.flush();
//            std::cout << "WaitEvent" << i << std::endl; std::cout.flush();

            ForwardMessages();

            RecvMessages();

            if (IsClosed()) {
                std::cout<< "Connection broken, exit." <<std::endl;
                return -1;
            }

        } catch (const std::exception& e) {

            std::cerr<< e.what() <<std::endl; std::cerr.flush();

        }

    }
}

