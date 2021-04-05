//
// Created by 田地 on 2021/3/11.
//

#include <chrono>
#include <cstring>
#include <poll.h>

#include "tftpserver.h"
#include "tftpsession.h"
#include "utils.h"

using namespace std::chrono;

TFTP::PollServer::PollServer(struct ServerConfig _serverConfig)
: serverConfig(_serverConfig),
  daemonSession(TFTP::Session::DaemonSession::Create(serverConfig.addr, serverConfig.port, serverConfig.backlog)) {}

TFTP::PollServer::~PollServer() {}

void TFTP::PollServer::Start() {
    daemonSession->Init();
    INFO("Server listen on: " + serverConfig.addr + ":" + std::to_string(serverConfig.port));

    while (true) {
        // init pollfd
        pollfd pollfds[requestSessions.size() + 1];
        pollfds[0] = {daemonSession->Socket(), POLLIN};

        for (int i = 0; i < requestSessions.size(); i++) {
            pollfds[i+1] = {requestSessions.at(i)->Socket(), POLLIN | POLLOUT};
        }

        int ret = poll(pollfds, sizeof(pollfds) / sizeof(pollfd),
            duration_cast<milliseconds>(serverConfig.timeout).count());

        if (ret < 0) {
            ERROR(std::string("poll: ") + strerror(errno));
            break;
        }

        // request sessions
        if (ret > 0) {
            for (int i = requestSessions.size()-1; i >= 0; i--) {

                bool readReady = pollfds[i+1].revents & POLLIN;
                bool writeReady = pollfds[i+1].revents & POLLOUT;

                auto& sess = requestSessions.at(i);

                if (readReady || writeReady) {
                    try {
                        sess->Proceeds(readReady, writeReady);
                    } catch (std::exception& e) {
                        ERROR(sess->ID(), std::string("Proceeds: ") + e.what());
                        sess->Terminate();
                    }
                }

                if (sess->Terminated()) {
                    INFO(sess->ID(),"session terminated: " + sess->ErrMsg());
                    requestSessions.erase(requestSessions.begin() + i);
                }
            }

            // daemon session
            if (pollfds[0].revents & POLLIN) {
                try {
                    auto req = daemonSession->AcceptRequest(serverConfig.timeout);
                    if (req) {
                        INFO("AcceptRequest: new session " + req->ID() +
                        ", path: " + req->Path() + ", mode: " + req->Mode());
                        requestSessions.emplace_back(std::move(req));
                        INFO("current session number: " + std::to_string(requestSessions.size()));
                    }
                } catch (std::exception& e) {
                    ERROR(std::string("AcceptRequest: ") + e.what());
                }
            }
        }

    }
}

