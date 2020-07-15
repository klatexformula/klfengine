cmake_minimum_required(VERSION 3.8)
# CMake >= 3.8 for CXX_STANDARD==17 target property

project(klfengine LANGUAGES CXX VERSION 0.1)



# integration with CTest
include(CTest)
enable_testing()

add_subdirectory(test)
