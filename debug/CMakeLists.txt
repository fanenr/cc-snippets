cmake_minimum_required(VERSION 3.14)
project(debug CXX)

cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED
  COMPONENTS
    stacktrace_noop
    stacktrace_basic
    stacktrace_addr2line
    stacktrace_backtrace
    stacktrace_from_exception
)

add_executable(trace trace.cc)

target_compile_definitions(trace
  PRIVATE
    # BOOST_STACKTRACE_USE_NOOP=1
    # BOOST_STACKTRACE_USE_BASIC=1
    # BOOST_STACKTRACE_USE_ADDR2LINE=1
    BOOST_STACKTRACE_USE_BACKTRACE=1

    BOOST_ENABLE_ASSERT_DEBUG_HANDLER=1
)

target_link_libraries(trace
  PRIVATE
    # Boost::stacktrace_noop
    # Boost::stacktrace_basic
    # Boost::stacktrace_addr2line
    Boost::stacktrace_backtrace
    Boost::stacktrace_from_exception
)
