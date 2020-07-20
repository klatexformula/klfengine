

find_package(Catch2 REQUIRED)

add_executable(testklfengine
  #
  # list sources here -->
  #
  test_basedefs.cxx
  test_value.cxx
  
  test_main.cxx # provides main() test function
)

set_property(TARGET testklfengine PROPERTY CXX_STANDARD 17)

# see also https://foonathan.net/2018/10/cmake-warnings/
set(KLFENGINE_TEST_WARN_OPTIONS "-Werror;-Wall;-Wextra"
  CACHE STRING "Compiler warning options to set")
target_compile_options(testklfengine PRIVATE ${KLFENGINE_TEST_WARN_OPTIONS})

# include directories etc. for <klfengine/xxxxx.h>

target_link_libraries(testklfengine PRIVATE klfengine)
target_link_libraries(testklfengine PRIVATE Catch2::Catch2)

add_test(NAME AllTests COMMAND testklfengine)
