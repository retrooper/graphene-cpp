cmake_minimum_required(VERSION 3.20)
project(graphene)

set(CMAKE_CXX_STANDARD 17)

include_directories(libs)
include_directories(include)
include_directories(include/encryption)
include(FetchContent)

file(GLOB sources "${PROJECT_SOURCE_DIR}/include/encryption/*.cpp")


add_executable(graphene ${sources} src/main.cpp)
FetchContent_Declare(fetch_nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json)

FetchContent_MakeAvailable(fetch_nlohmann_json)

target_link_libraries(graphene nlohmann_json)

FetchContent_Declare(fetch_mbedtls
        GIT_REPOSITORY https://github.com/ARMmbed/mbedtls)

FetchContent_MakeAvailable(fetch_mbedtls)

target_link_libraries(graphene mbedtls)