cmake_minimum_required(VERSION 3.14)
project(echo CXX)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED COMPONENTS context)

add_executable(server server.cc)
target_compile_definitions(server
  PRIVATE
    BOOST_ASIO_HAS_IO_URING=1
    BOOST_ASIO_DISABLE_EPOLL=1
)
target_link_libraries(server PRIVATE uring)

add_executable(client client.cc)
target_compile_definitions(client
  PRIVATE
    BOOST_ASIO_HAS_IO_URING=1
    BOOST_ASIO_DISABLE_EPOLL=1
)
target_link_libraries(client PRIVATE uring)

add_executable(co_server co_server.cc)
target_link_libraries(co_server PRIVATE Boost::context)

add_executable(co_client co_client.cc)
target_link_libraries(co_client PRIVATE Boost::context)
