cmake_minimum_required(VERSION 3.12)
aux_source_directory(./src COMPILER_SRC)
include_directories(./include)
add_library(${NEO_PROJECT_NAME}-compiler STATIC ${COMPILER_SRC})