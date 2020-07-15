

find_package(Catch2 REQUIRED)

add_executable(testklfengine
  test_input.cxx
  test_main.cxx
)

set_property(TARGET testklfengine PROPERTY CXX_STANDARD 17)

# see also https://foonathan.net/2018/10/cmake-warnings/
set(KLFENGINE_TEST_WARN_OPTIONS "-Werror;-Wall;-Wextra"
  CACHE STRING "Compiler warning options to set")
target_compile_options(testklfengine PRIVATE ${KLFENGINE_TEST_WARN_OPTIONS})

# include directory for <klfengine/xxxxx.h>
target_include_directories(
  testklfengine
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
  )

target_link_libraries(testklfengine PRIVATE Catch2::Catch2)

add_test(NAME AllTests COMMAND testklfengine)
