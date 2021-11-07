cmake_minimum_required(VERSION 3.20)
project(graphene)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/DevLibraries/boost_1_77_0_b1_rc1")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/DevLibraries/boost_1_77_0_b1_rc1/libs")
include_directories(libs)
include_directories(include)
include(FetchContent)



add_executable(graphene src/main.cpp)

FetchContent_Declare(fetch_nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json)

FetchContent_MakeAvailable(fetch_nlohmann_json)

target_link_libraries(graphene nlohmann_json)

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()