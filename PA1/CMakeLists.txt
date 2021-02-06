cmake_minimum_required(VERSION 3.10)
project(proj1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output/)

add_executable(server server.cpp EchoServer.h EchoServer.cpp iosocket.h iosocket.cpp)

add_executable(client client.cpp EchoClient.h EchoClient.cpp iosocket.h iosocket.cpp)