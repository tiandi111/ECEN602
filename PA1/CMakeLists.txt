cmake_minimum_required(VERSION 3.10)
project(proj1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output/)

add_executable(server server.cpp echoserver.h echoserver.cpp iosocket.h iosocket.cpp)

add_executable(client client.cpp echoclient.h echoclient.cpp iosocket.h iosocket.cpp)