cmake_minimum_required(VERSION 3.18)
project(netAssignment)

set(CMAKE_CXX_STANDARD 17)

add_executable(netAssignment
        main.cpp
        Server.cpp
        Server.h
        ChatRoom.cpp
        ChatRoom.h
        Request.h
        Response.h
        utils.cpp
        utils.h
        )