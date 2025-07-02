cmake_minimum_required(VERSION 3.10)
project(reMarkable-tablet-driver C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED libssh)

include_directories(${LIBSSH_INCLUDE_DIRS})
link_directories(${LIBSSH_LIBRARY_DIRS})

add_executable(rmTabletDriver
    src/tabletDriver.c
    src/argument_parser.c
    src/ssh.c
)

target_link_libraries(rmTabletDriver ${LIBSSH_LIBRARIES})

install(TARGETS rmTabletDriver DESTINATION bin)
