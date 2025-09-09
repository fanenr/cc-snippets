cmake_minimum_required(VERSION 3.14)
project(coro CXX)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED COMPONENTS context)

add_executable(stackfull stackfull.cc)
target_link_libraries(stackfull PRIVATE Boost::context)

# add_executable(stackless stackless.cc)
# target_link_libraries(stackless PRIVATE Boost::context)
