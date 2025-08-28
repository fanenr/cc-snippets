cmake_minimum_required(VERSION 3.21)
project(echo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS context)

add_executable(server server.cc)
target_link_libraries(server Boost::context)

add_executable(client co_client.cc)
target_link_libraries(client Boost::context)
