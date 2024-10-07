cmake_minimum_required(VERSION 3.29)
project(Simple_Chatting C)

set(CMAKE_C_STANDARD 11)

add_executable(Simple_Chatting
        server.c
        control.c
        client.c
        simple_chatting.h
        )
