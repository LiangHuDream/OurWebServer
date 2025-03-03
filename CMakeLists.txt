cmake_minimum_required(VERSION 3.10)

project(OurWebServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-O2 -Wall -g")

add_subdirectory(test)
add_subdirectory(webbench-1.5)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)