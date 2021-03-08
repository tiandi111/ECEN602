cmake_minimum_required(VERSION 3.10)
project(PA2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output/)

add_executable(server server.cpp protocol.h protocol.cpp sbcpserver.cpp sbcpserver.h io.h io.cpp)

add_executable(client client.cpp sbcpclient.h sbcpclient.cpp protocol.h protocol.cpp io.h io.cpp)