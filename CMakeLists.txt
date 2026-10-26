cmake_minimum_required(VERSION 3.16)
project(pagerank_distributed LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pagerank
    src/main.cpp
    src/graph_io.cpp
    src/pagerank.cpp
)

target_include_directories(pagerank PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
