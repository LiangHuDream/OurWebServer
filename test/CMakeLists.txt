file(GLOB_RECURSE SOURCES 
    ../code/log/*.cpp 
    ../code/pool/*.cpp 
    ../code/timer/*.cpp 
    ../code/http/*.cpp 
    ../code/server/*.cpp 
    ../code/buffer/*.cpp 
    ../test/test.cpp
)

add_executable(test ${SOURCES})

target_link_libraries(test pthread mysqlclient)