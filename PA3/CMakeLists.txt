cmake_minimum_required(VERSION 3.10)
project(PA3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output/)
set(CMAKE_CXX_COMPILER "g++-10") # for local test only

add_executable(server
        server.cpp
        tftpserver.cpp
        tftpserver.h
        tftpsession.cpp
        tftpsession.h
        utils.h
        utils.cpp
        io.h
        io.cpp)

add_compile_options(-Wno-unused-value)

add_executable(test
        test/unittest.cpp
        test/unittest.h
        test/mocklib.cpp
        tftpsession.cpp
        tftpsession.h
        io.cpp
        io.h
        utils.h)

