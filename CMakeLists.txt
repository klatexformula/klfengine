cmake_minimum_required(VERSION 3.8)
# CMake >= 3.XXX? for "linking" object libraries in test subdir
# ( CMake >= 3.8 for "cxx_std_17" as C++ known feature )

# It seems using FetchContent would be a good idea for dealing with external
# dependencies (as opposed to my crappy shell script), but I can't seem to
# figure out a good way to integrate it with find_package() (e.g., only download
# if a system version isn't found, etc.)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(klfengine_get_version)

#
# CMAKE project() DIRECTIVE HERE.
#
project(klfengine LANGUAGES CXX VERSION "${KLFENGINE_VERSION}")


#
# Some initial variable settings, place in cache
#
set(KLFENGINE_TESTS false CACHE STRING "Build tests")

if("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  # compiler supports C++17
  set(_klfe_default_use_mpark_variant false)
else()
  set(_klfe_default_use_mpark_variant true)
endif()

set(KLFENGINE_USE_MPARK_VARIANT "${_klfe_default_use_mpark_variant}" CACHE BOOL
  "Whether or not to use mpark/variant instead of C++17 std::variant")

set(KLFENGINE_USE_GULRAK_FILESYSTEM ON CACHE BOOL
  "Use gulrak/filesystem instead of C++17 std::filesystem")

set(KLFENGINE_USE_LINKED_GHOSTSCRIPT OFF CACHE BOOL
  "klfengine: Whether to compile against ghostscript's C API library")

set(KLFENGINE_USE_LOAD_GHOSTSCRIPT OFF CACHE BOOL
  "klfengine: Whether to enable loading ghostscript's libgs at runtime (need ghostscript headers at compile-time)")


#
# Welcome & show those variable values
#
message(STATUS "Welcome to klfengine's CMake configuration script (${KLFENGINE_VERSION})

These CMake scripts will let you:

  - install headers at your preferred location

    --> after successful CMake run, use \"make install\".  (Set
        CMAKE_INSTALL_PREFIX as desired.)

  - install CMake configuration files to easily set up other
    CMake-based projects to use klfengine

    --> installed as part of \"make install\" (see above)

  - build and run the tests

    --> set KLFENGINE_TESTS=true, then \"make\", then \"make test\"

Some important CMake variables to set are:

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

  - KLFENGINE_USE_MPARK_VARIANT=true|false  (& others.....)

    [current value: ${KLFENGINE_USE_MPARK_VARIANT}]

    Whether to use the alternative library mpark-variant instead of
    C++17's std::variant.  This does not make much sense if your
    compiler supports C++17, but it is necessary if you are compiling
    in C++11 or C++14 mode.  If set to true, you should make sure you
    have the mpark/variant library installed.

    CMake projects that import klfengine will use the provided value as the
    default; they can still override this choice by setting this variable before
    calling find_package(klfengine...).

  - KLFENGINE_USE_GULRAK_FILESYSTEM=true|false

    Whether to use the alternative library gulrak/filesystem instead of
    C++17's std::filesystem.  This does not make much sense if your
    compiler supports C++17, but it is necessary if you are compiling
    in C++11 or C++14 mode.  If set to true, you should make sure you
    have the gulrak/filesystem library installed.

    CMake projects that import klfengine will use the provided value as the
    default; they can still override this choice by setting this variable before
    calling find_package(klfengine...).

  - KLFENGINE_USE_LINKED_GHOSTSCRIPT=true|false

    Whether to link to ghostscript's C API library at compile time. This might
    speed up calls to ghostscript as we don't have to start a new process at
    each call.

    CMake projects that import klfengine will use the provided value as the
    default; they can still override this choice by setting this variable before
    calling find_package(klfengine...).

  - KLFENGINE_USE_LOAD_GHOSTSCRIPT=true|false

    Prepare klfengine's sources so that it can load libgs.so at runtime to run
    ghostscript without executing new processes.  this option is similar to
    KLFENGINE_USE_LINKED_GHOSTSCRIPT, except that the sources are not explicitly
    linked to libgs.

    CMake projects that import klfengine will use the provided value as the
    default; they can still override this choice by setting this variable before
    calling find_package(klfengine...).

Other variables that you can set will be specified in status messages
below.
")



include(GNUInstallDirs)
include(CMakePackageConfigHelpers)



#
# ### in progress -- TODO: SAVE found dependencies to the installed cmake target files
#

set(_klfengine_no_msg_prefix TRUE)
include(klfengine_setup_dependencies)

# find_package(nlohmann_json REQUIRED)

# message(STATUS "Using nlohmann_json at ${nlohmann_json_DIR} (nlohmann_json_DIR)")

# if(KLFENGINE_USE_MPARK_VARIANT)
#   message(STATUS
#     "Will use mpark/variant instead of C++17 std::variant (KLFENGINE_USE_MPARK_VARIANT)")
#   find_package(mpark_variant REQUIRED)
#   message(STATUS "Using mpark_variant at ${mpark_variant_DIR} (mpark_variant_DIR)")
# else()
#   message(STATUS
#     "Will use C++17's std::variant (KLFENGINE_USE_MPARK_VARIANT)")
# endif()

# if(KLFENGINE_USE_GULRAK_FILESYSTEM)
#   message(STATUS
#     "Will use gulrak/filesystem and not std::filesystem (KLFENGINE_USE_GULRAK_FILESYSTEM)")
#   find_package(ghcFilesystem REQUIRED)
#   message(STATUS "Using gulrak/filesystem at ${ghcFilesystem_DIR} (ghcFilesystem_DIR)")
# else()
#   message(STATUS
#     "Will use C++17's std::filesystem (KLFENGINE_USE_GULRAK_FILESYSTEM)")
# endif()



add_library(klfengine INTERFACE)

target_include_directories(klfengine
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )
# target_link_libraries(klfengine
#   INTERFACE
#   nlohmann_json::nlohmann_json
#   )
# if(KLFENGINE_USE_MPARK_VARIANT)
#   target_link_libraries(klfengine
#     INTERFACE
#     mpark_variant
#     )
#   target_compile_options(klfengine
#     INTERFACE
#     -DKLFENGINE_USE_MPARK_VARIANT
#     )
# endif()
# if(KLFENGINE_USE_GULRAK_FILESYSTEM)
#   target_link_libraries(klfengine
#     INTERFACE
#     ghcFilesystem::ghc_filesystem
#     )
#   target_compile_options(klfengine
#     INTERFACE
#     -DKLFENGINE_USE_GULRAK_FILESYSTEM
#     )
# endif()

set(_klfengine_target_name "klfengine")
include(klfengine_set_deps_interface_tgt_prop)


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

file(READ "${CMAKE_SOURCE_DIR}/cmake/klfengine_setup_dependencies.cmake"
  KLFENGINE_CODE_CMAKE_SETUP_DEPENDENCIES)
file(READ "${CMAKE_SOURCE_DIR}/cmake/klfengine_set_deps_interface_tgt_prop.cmake"
  KLFENGINE_CODE_SET_DEPS_INTERFACE_TARGET_PROPERTIES)
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

  message(STATUS "Will build tests (KLFENGINE_TESTS)")

  # use CTest

  include(CTest)
  enable_testing()

  add_subdirectory(test)

else()

  message(STATUS "Skipping tests (KLFENGINE_TESTS)")

endif()
