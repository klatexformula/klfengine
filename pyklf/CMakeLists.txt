cmake_minimum_required(VERSION 3.9)

project(pyklf)

find_package(pybind11 2.4 REQUIRED)

pybind11_add_module(pyklf cxx/pyklf.cxx)

set_target_properties(pyklf PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQURIED on
  )

target_compile_definitions(pyklf PRIVATE -DKLFENGINE_USE_GULRAK_FILESYSTEM)

target_include_directories(pyklf PRIVATE ../deps_install/include ../include)
