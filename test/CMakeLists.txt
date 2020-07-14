

find_package(Catch2 REQUIRED)

add_executable(testklfengine
  test_input.cxx
)

set_property(TARGET testklfengine PROPERTY CXX_STANDARD 17)

target_include_directories(testklfengine
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..) # <klfengine/xxxxx.h>

target_link_libraries(testklfengine PRIVATE Catch2::Catch2)




include(CTest)
include(Catch)
catch_discover_tests(testklfengine)
