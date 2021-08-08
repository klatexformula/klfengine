# ------------------------------------------------------------------------------
# Find necessary dependencies for compiling klfengine into a user project.
# ------------------------------------------------------------------------------

if(_klfengine_no_msg_prefix)
  set(_klfengine_msg "")
else()
  set(_klfengine_msg "klfengine: ")
endif()

#
# nlohmannn/json
#

find_package(nlohmann_json REQUIRED
  CONFIG
  PATHS "${_klfengine_cmake_default_nlohmann_json_DIR}"
  )
message(STATUS "${_klfengine_msg}Using nlohmann_json at ${nlohmann_json_DIR} (nlohmann_json_DIR)")


#
# mpark/variant
#

if(KLFENGINE_USE_MPARK_VARIANT)
  message(STATUS
    "${_klfengine_msg}Will use mpark/variant and not std::variant (KLFENGINE_USE_MPARK_VARIANT)")
  find_package(mpark_variant REQUIRED
    CONFIG
    PATHS "${_klfengine_cmake_default_mpark_variant_DIR}"
    )
  message(STATUS "${_klfengine_msg}Using mpark_variant at ${mpark_variant_DIR} (mpark_variant_DIR)")
else()
  message(STATUS
    "${_klfengine_msg}Will use C++17's std::variant (KLFENGINE_USE_MPARK_VARIANT)")
endif()

#
# gulrak/filesystem
#

if(KLFENGINE_USE_GULRAK_FILESYSTEM)
  message(STATUS
    "${_klfengine_msg}Will use gulrak/filesystem and not std::filesystem (KLFENGINE_USE_GULRAK_FILESYSTEM)")
  find_package(ghcFilesystem REQUIRED
    CONFIG
    PATHS "${_klfengine_cmake_default_ghcFilesystem_DIR}"
    )
  message(STATUS
    "${_klfengine_msg}Using gulrak/filesystem at ${ghcFilesystem_DIR} (ghcFilesystem_DIR)")
else()
  message(STATUS
    "${_klfengine_msg}Will use C++17's std::filesystem (KLFENGINE_USE_GULRAK_FILESYSTEM)")
endif()

#
# ghostscript headers (set GHOSTSCRIPT_ROOT_DIR as hint)
#

if(KLFENGINE_USE_LINKED_GHOSTSCRIPT)
  message(STATUS
    "${_klfengine_msg}Will link against Ghostscript's C API library (KLFENGINE_USE_LINKED_GHOSTSCRIPT)")
endif()
if(KLFENGINE_USE_LOAD_GHOSTSCRIPT)
  message(STATUS
    "${_klfengine_msg}Will prepare to load Ghostscript's libgs at runtime (KLFENGINE_USE_LOAD_GHOSTSCRIPT)")
endif()
if(NOT KLFENGINE_USE_LINKED_GHOSTSCRIPT AND NOT KLFENGINE_USE_LOAD_GHOSTSCRIPT)
  message(STATUS
    "${_klfengine_msg}Will NOT use Ghostscript's C API library (KLFENGINE_USE_LINKED_GHOSTSCRIPT,KLFENGINE_USE_LOAD_GHOSTSCRIPT)")
endif()



if(KLFENGINE_USE_LINKED_GHOSTSCRIPT OR KLFENGINE_USE_LOAD_GHOSTSCRIPT)
  find_path(
    GHOSTSCRIPT_INCLUDE
    NAMES ghostscript/iapi.h ghostscript/ierrors.h
    HINTS
      "${_klfengine_cmake_default_GHOSTSCRIPT_INCLUDE}"
      "${GHOSTSCRIPT_ROOT_DIR}/include"
      "/usr/local/opt/ghostscript/include"
      # search user project's source directory for ghostscript/xxx.h or include/ghostscript/xxx.h
      "${CMAKE_SOURCE_DIR}"
      "${CMAKE_SOURCE_DIR}/include"
    )
  message(STATUS
    "${_klfengine_msg}Ghostscript headers located at ${GHOSTSCRIPT_INCLUDE} (GHOSTSCRIPT_ROOT_DIR,GHOSTSCRIPT_INCLUDE)")
endif()

#
# ghostscript library
#

if(KLFENGINE_USE_LINKED_GHOSTSCRIPT)
  find_library(
    GHOSTSCRIPT_LIB
    NAMES gs libgs gsdll
    HINTS
      "${_klfengine_cmake_default_GHOSTSCRIPT_LIB}"
      "${GHOSTSCRIPT_ROOT_DIR}/lib"
      "/usr/local/opt/ghostscript/lib"
  )
  message(STATUS
    "${_klfengine_msg}Ghostscript library located at ${GHOSTSCRIPT_LIB} (GHOSTSCRIPT_ROOT_DIR,GHOSTSCRIPT_LIB)")
endif()
