cmake_minimum_required(VERSION 3.12)
project(neo)
set(NEO_PROJECT_NAME ${PROJECT_NAME})
include_directories(.)
add_subdirectory(./engine)
add_subdirectory(./vm)
add_subdirectory(./common)

add_executable(${NEO_PROJECT_NAME} ./main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE ${NEO_PROJECT_NAME}-engine ${NEO_PROJECT_NAME}-vm ${NEO_PROJECT_NAME}-common)

enable_testing()
add_subdirectory(./tests)
