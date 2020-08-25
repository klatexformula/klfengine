cmake_minimum_required(VERSION 3.9)

project(pyklf)

find_package(pybind11 2.4 REQUIRED)

pybind11_add_module(_cxx_pyklf cxx/pyklf.cxx)

set_target_properties(_cxx_pyklf PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQURIED on
  )

target_include_directories(_cxx_pyklf PRIVATE ../deps_install/include ../include)
