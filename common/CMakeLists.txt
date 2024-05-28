cmake_minimum_required(VERSION 3.12)
aux_source_directory(./src COMMON_SRC)
include_directories(./include)
add_library(${NEO_PROJECT_NAME}-common SHARED ${COMMON_SRC})
