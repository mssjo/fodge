cmake_minimum_required(VERSION 3.10)
project(fodge)

file(GLOB SOURCES "src/*.cpp")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

include_directories(include)

add_executable(fodge ${SOURCES})
