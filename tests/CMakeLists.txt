cmake_minimum_required(VERSION 3.12)
project(test)
add_executable(TestCall TestCall.c)
target_link_libraries(TestCall PRIVATE ${NEO_SCRIPT_NAME})