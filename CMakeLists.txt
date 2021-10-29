cmake_minimum_required(VERSION 3.20)
project(graphene)

set(CMAKE_CXX_STANDARD 17)

include_directories(libs)
include_directories(include)
include(FetchContent)

file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(graphene ${sources} main.cpp)

FetchContent_Declare(fetch_nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json)

FetchContent_MakeAvailable(fetch_nlohmann_json)

target_link_libraries(graphene nlohmann_json )