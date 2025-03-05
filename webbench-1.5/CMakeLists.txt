set(CMAKE_C_FLAGS "-Wall -ggdb -W -O")

file(GLOB_RECURSE SOURCES *.cpp)
include_directories(/usr/include/tirpc)

add_executable(webbench webbench.cpp socket.cpp)

install(TARGETS webbench
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

install(FILES webbench.1
    DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)

install(DIRECTORY debian/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/webbench
    FILES_MATCHING PATTERN "copyright" PATTERN "changelog"
)