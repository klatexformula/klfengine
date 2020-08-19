
find_package(Catch2 REQUIRED)
message(STATUS "Using Catch2 at ${Catch2_DIR} (Catch2_DIR)")


# see also https://foonathan.net/2018/10/cmake-warnings/
set(KLFENGINE_TEST_WARN_OPTIONS "-Werror;-Wall;-Wextra"
  CACHE STRING "Compiler warning options to set")

message(STATUS
  "Compiler warnings for tests are \"${KLFENGINE_TEST_WARN_OPTIONS}\" (KLFENGINE_TEST_WARN_OPTIONS)")


## FIXME: Worry later about compatibility with C++11, do C++17 for now (for std::variant)
set(KLFENGINE_TEST_CXX_STANDARD "17"
  CACHE STRING "Which C++ standard(s) to compile tests with (\"17\" -> C++17)")
message(STATUS
  "Compiling tests with C++${KLFENGINE_TEST_CXX_STANDARD} (KLFENGINE_TEST_CXX_STANDARD=11|14|17)")


add_library(test_main OBJECT test_main.cxx)
target_compile_options(test_main PRIVATE ${KLFENGINE_TEST_WARN_OPTIONS})
set_target_properties(test_main
  PROPERTIES
    CXX_STANDARD ${KLFENGINE_TEST_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON)
target_link_libraries(test_main PRIVATE
  klfengine
  Catch2::Catch2
  )

#
# klfengine_create_test(xyz
#   SOURCES test_x.cxx test_y.cxx test_z.cxx
#   LIBRARIES <extra library targets to link this test to>
#   COMPILE_OPTIONS <extra compile options for this test>
# )
#
macro(klfengine_create_test test_name)
  set(options )
  set(oneValueArgs )
  set(multiValueArgs SOURCES LIBRARIES COMPILE_OPTIONS)
  cmake_parse_arguments(KLFENGINE_CREATE_TEST "${options}" "${oneValueArgs}"
    "${multiValueArgs}" ${ARGN})
  # --> KLFENGINE_CREATE_TEST_SOURCES, KLFENGINE_CREATE_TEST_LIBRARIES, etc.

  add_executable("test_${test_name}"
    # sources
    ${KLFENGINE_CREATE_TEST_SOURCES}
    $<TARGET_OBJECTS:test_main>  # test_main.cxx
    )

  target_compile_options("test_${test_name}" PRIVATE
    ${KLFENGINE_TEST_WARN_OPTIONS}
    ${KLFENGINE_CREATE_TEST_COMPILE_OPTIONS}
    "-DKLFENGINE_TEST_ROOT_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
    )

  set_target_properties("test_${test_name}"
    PROPERTIES
      CXX_STANDARD ${KLFENGINE_TEST_CXX_STANDARD}
      CXX_STANDARD_REQUIRED ON)

  # include directories etc. for <klfengine/xxx>.  Here in CMake, "linking"
  # against klfengine automatically links against nlohmann_json and other direct
  # dependencies.

  target_link_libraries("test_${test_name}" PRIVATE
    klfengine
    Catch2::Catch2
    Threads::Threads
    ${KLFENGINE_CREATE_TEST_LIBRARIES}
    )

  add_test(NAME "test_${test_name}"
    COMMAND "test_${test_name}")

endmacro()


#
# List all test sources here:
#

klfengine_create_test(basedefs SOURCES test_basedefs.cxx)

klfengine_create_test(value SOURCES test_value.cxx)

klfengine_create_test(version SOURCES test_version.cxx)

klfengine_create_test(input SOURCES test_input.cxx)

klfengine_create_test(settings SOURCES test_settings.cxx)

klfengine_create_test(format_spec SOURCES test_format_spec.cxx)

klfengine_create_test(engine SOURCES test_engine.cxx)

klfengine_create_test(engine_run_implementation
  SOURCES test_engine_run_implementation.cxx)

klfengine_create_test(run SOURCES test_run.cxx)

klfengine_create_test(process SOURCES test_process.cxx)

klfengine_create_test(temporary_directory
  SOURCES test_temporary_directory.cxx)

klfengine_create_test(klfimplpkg_engine_run_implementation
  SOURCES test_klfimplpkg_engine_run_implementation.cxx)

klfengine_create_test(klfimplpkg_engine_engine
  SOURCES test_klfimplpkg_engine_engine.cxx)

klfengine_create_test(latex_dvips_gs_engine_run_implementation
  SOURCES test_latex_dvips_gs_engine_run_implementation.cxx)

klfengine_create_test(latex_dvips_gs_engine_engine
  SOURCES test_latex_dvips_gs_engine_engine.cxx)

klfengine_create_test(detail_filesystem
  SOURCES test_detail_filesystem.cxx)

klfengine_create_test(detail_simple_gs_interface
  SOURCES test_detail_simple_gs_interface.cxx)



klfengine_create_test(separate_impl
  SOURCES test_separate_impl.cxx
          #
          test_basedefs.cxx
          test_value.cxx
          test_version.cxx
          test_input.cxx
          test_settings.cxx
          test_format_spec.cxx
          test_engine_run_implementation.cxx
          test_engine.cxx
          test_run.cxx
          test_process.cxx
          test_temporary_directory.cxx
          test_klfimplpkg_engine_run_implementation.cxx
          test_klfimplpkg_engine_engine.cxx
          test_latex_dvips_gs_engine_run_implementation.cxx
          test_latex_dvips_gs_engine_engine.cxx
           #
          test_detail_filesystem.cxx
          test_detail_simple_gs_interface.cxx
          
  COMPILE_OPTIONS
          -DKLFENGINE_SEPARATE_IMPLEMENTATION
)
