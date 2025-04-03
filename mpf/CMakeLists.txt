cmake_minimum_required(VERSION 3.20)
project(mini_mpf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_subdirectory(test)

add_custom_target(
    build
    COMMAND cmake ..
)

add_custom_target(
    run_t
    COMMAND cmake --build .
    COMMAND ./test/test
    DEPENDS test
)