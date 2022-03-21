project(webserver)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread")

cmake_minimum_required(VERSION 3.5)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} main.cpp Buffer.cpp Epoller.cpp Time.cpp HTTPRequest.cpp HTTPResponse.cpp HTTPConnection.cpp webserver.cpp)
