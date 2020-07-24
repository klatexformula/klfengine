cmake_minimum_required(VERSION 3.8)
# CMake >= 3.8 for CXX_STANDARD==17 target property

project(klfengine LANGUAGES CXX VERSION 0.1)



find_package(nlohmann_json REQUIRED)


add_library(klfengine INTERFACE)

target_include_directories(klfengine INTERFACE ./)
target_link_libraries(klfengine INTERFACE nlohmann_json::nlohmann_json)


# integration with CTest
include(CTest)
enable_testing()

add_subdirectory(test)
