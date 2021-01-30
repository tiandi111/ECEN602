cmake_minimum_required(VERSION 3.10)
project(proj1)

set(CMAKE_CXX_STANDARD 11)

add_executable(server server.cpp EchoServer.h EchoServer.cpp utils.h utils.cpp Reader.h Reader.cpp)

add_executable(client client.cpp EchoClient.h EchoClient.cpp utils.h utils.cpp Reader.h Reader.cpp)