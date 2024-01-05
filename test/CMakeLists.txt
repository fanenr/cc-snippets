cmake_minimum_required(VERSION 3.20)
project(trash)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
add_compile_options(-fmodules-ts)


aux_source_directory(. src)
aux_source_directory(./module src_module)
add_executable(main ${src_module} ${src})
# add_executable(main fmt.cpp)


find_package(Catch2 REQUIRED)
target_link_libraries(main PRIVATE Catch2WithMain)
