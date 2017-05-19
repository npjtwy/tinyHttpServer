cmake_minimum_required(VERSION 3.6)
project(HttpServer)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DBOOST_LOG_DYN_LINK)

set(SOURCE_FILES server_main.cpp HttpHandle.cpp HttpHandle.h
        HttpServer.h HttpServer.cpp MySoket.h MySoket.cpp
        MysqlHelper.h MysqlHelper.cpp log/Logger.cpp log/Logger.h)

link_libraries(-lpthread -lmysqlclient -lboost_system -lboost_log -lboost_thread
        -lboost_log_setup)
add_executable(HttpServer ${SOURCE_FILES})