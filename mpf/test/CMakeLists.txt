project(mini_mpf_test)

aux_source_directory(. test_src)
add_executable(test ${test_src})

target_include_directories(test PRIVATE ../include)