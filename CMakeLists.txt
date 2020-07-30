cmake_minimum_required(VERSION 3.8)
# CMake >= 3.8 for CXX_STANDARD==17 target property

project(klfengine LANGUAGES CXX VERSION 0.1)



#
# Read and parse current header version.  (We parse the header instead
# of having the value in CMake and generating a header with version
# info, because we'd like people to able to use the raw headers
# directly from the repo without running any build scripts.)
#
file(READ "include/klfengine/h/version_number_raw.h" _klfe_version_raw_header)
string(REGEX MATCH "#define KLFENGINE_VERSION_MAJOR [0-9]+"
  _klfe_version_macrodef "${_klfe_version_raw_header}")
string(REGEX MATCH "[0-9]+" KLFENGINE_VERSION_MAJOR "${_klfe_version_macrodef}")
string(REGEX MATCH "#define KLFENGINE_VERSION_MINOR [0-9]+"
  _klfe_version_macrodef "${_klfe_version_raw_header}")
string(REGEX MATCH "[0-9]+" KLFENGINE_VERSION_MINOR "${_klfe_version_macrodef}")
string(REGEX MATCH "#define KLFENGINE_VERSION_RELEASE [0-9]+"
  _klfe_version_macrodef "${_klfe_version_raw_header}")
string(REGEX MATCH "[0-9]+" KLFENGINE_VERSION_RELEASE "${_klfe_version_macrodef}")
string(REGEX MATCH "#define KLFENGINE_VERSION_SUFFIX \"[^\"]*\""
  _klfe_version_macrodef "${_klfe_version_raw_header}")
string(REGEX REPLACE "#define KLFENGINE_VERSION_SUFFIX \"([^\"]*)\""  "\\1"
  KLFENGINE_VERSION_SUFFIX "${_klfe_version_macrodef}")

set(KLFENGINE_VERSION "${KLFENGINE_VERSION_MAJOR}.${KLFENGINE_VERSION_MINOR}.${KLFENGINE_VERSION_RELEASE}${KLFENGINE_VERSION_SUFFIX}")


set(KLFENGINE_TESTS true CACHE STRING "Build tests")


message(STATUS "Welcome to klfengine's CMake configuration script (${KLFENGINE_VERSION})

These CMake scripts will let you:

  - install headers at your preferred location

  - install CMake configuration files to easily set up other
    CMake-based projects to use klfengine

  - build and run the tests

Relevant CMake variables to set are:

  - KLFENGINE_TESTS=true|false  [current value: ${KLFENGINE_TESTS}]

    Whether to build the tests or not.  If set to true, then you can
    use 'make' to compile the tests and 'make test' to run the tests.

  - CMAKE_INSTALL_PREFIX=/path/to/installation/prefix

    [current value: ${CMAKE_INSTALL_PREFIX}]

    Install location of the klfengine headers.  The folder structure
    \"include/klfengine/...\" will be created at the given location.

  - CMAKE_PREFIX_PATH=/some/path;/another/path;...

    [current value: ${CMAKE_PREFIX_PATH}]

    Install prefixes where we can find CMake configurations of
    installed dependencies.  See CMake's documentation for this
    variable.

")




include(GnuInstallDirs)
include(CMakePackageConfigHelpers)




find_package(nlohmann_json REQUIRED)

set(KLFENGINE_USE_MPARK_VARIANT off CACHE BOOL
  "Whether or not to use mpark/variant instead of C++17 std::variant")
if(KLFENGINE_USE_MPARK_VARIANT)
  find_package(mpark_variant REQUIRED)
endif()


add_library(klfengine INTERFACE)

target_include_directories(klfengine
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )
target_link_libraries(klfengine
  INTERFACE
  nlohmann_json::nlohmann_json
  )
if(KLFENGINE_USE_MPARK_VARIANT)
  target_link_libraries(klfengine
    INTERFACE
    mpark_variant
    )
  target_compile_options(klfengine
    INTERFACE
    -DKLFENGINE_USE_MPARK_VARIANT
    )
endif()

install(
  DIRECTORY     include/
  DESTINATION   "${CMAKE_INSTALL_INCLUDEDIR}"
  FILES_MATCHING PATTERN REGEX "^[a-zA-Z_0-9/-]+(\\.h(xx)?)?$"
)

install(
  TARGETS       klfengine
  EXPORT        klfengineTargets
  DESTINATION   "${CMAKE_INSTALL_LIBDIR}"
)

set(KLFENGINE_CMAKE_CONFIG_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/klfengine")

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/klfengineConfig.cmake.in
  ${PROJECT_BINARY_DIR}/klfengineConfig.cmake
  INSTALL_DESTINATION
  "${KLFENGINE_CMAKE_CONFIG_DESTINATION}"
  )

# A few lines of code are adapted from Catch2 -- see comments there:
# https://github.com/catchorg/Catch2/blob/229cc48/CMakeLists.txt#L140
#
set(_KLFE_TMP_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
set(CMAKE_SIZEOF_VOID_P "")
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/klfengineConfigVersion.cmake"
  VERSION ${KLFENGINE_VERSION}
  COMPATIBILITY SameMajorVersion
  )
set(CMAKE_SIZEOF_VOID_P ${_KLFE_TMP_CMAKE_SIZEOF_VOID_P})


install(
  EXPORT       klfengineTargets
  NAMESPACE    klfengine::
  DESTINATION  "${KLFENGINE_CMAKE_CONFIG_DESTINATION}"
  )
install(
  FILES        "${PROJECT_BINARY_DIR}/klfengineConfig.cmake"
               "${PROJECT_BINARY_DIR}/klfengineConfigVersion.cmake"
  DESTINATION  "${KLFENGINE_CMAKE_CONFIG_DESTINATION}"
)



# add uninstall target -- see
# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()




if(KLFENGINE_TESTS)

  # use CTest

  include(CTest)
  enable_testing()

  add_subdirectory(test)

endif()
