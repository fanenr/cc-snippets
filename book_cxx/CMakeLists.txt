cmake_minimum_required(VERSION 3.24)
project(cxx_examples
    VERSION 0.0.1
    LANGUAGES CXX)

# --------------------------------------------
# configure
# --------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")


# --------------------------------------------
# build mode
# --------------------------------------------
option(IS_DEBUG "is debug mode" on)
if (NOT IS_DEBUG)
    set(CMAKE_BUILD_TYPE "Release")
else()
    set(CMAKE_BUILD_TYPE "Debug")
endif()


# --------------------------------------------
# sub directory
# --------------------------------------------
set(end 3)

foreach(i RANGE 1 ${end})
    add_subdirectory("chapter${i}")
endforeach()