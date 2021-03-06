cmake_minimum_required(VERSION 3.10)
project(PA2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output/)

add_executable(server server.cpp sbcpserver.h sbcpserver.cpp protocol.h protocol.cpp io.h io.cpp)

add_executable(client client.cpp sbcpclient.h sbcpclient.cpp protocol.h protocol.cpp io.h io.cpp)

add_executable(test test/unittest.cpp protocol.h protocol.cpp io.h io.cpp)