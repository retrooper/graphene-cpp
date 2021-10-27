cmake_minimum_required(VERSION 3.20)
project(graphene)

set(CMAKE_CXX_STANDARD 17)

include_directories(libs)
include_directories(include)

file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(graphene ${sources} main.cpp)