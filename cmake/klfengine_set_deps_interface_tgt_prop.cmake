#
# Set target properties INTERFACE_COMPILE_OPTIONS, INTERFACE_LINK_LIBRARIES, and
# INTERFACE_INCLUDE_DIRECTORIES on the klfengine target, according to the user's
# options for dependencies (e.g. mpark/variant, etc.).  The target we need to
# set up is called ${_klfengine_target_name}.
#

get_target_property(
  _klfengine_interface_tgt_prop_compile_options
  ${_klfengine_target_name} INTERFACE_COMPILE_OPTIONS
  )
if(NOT _klfengine_interface_tgt_prop_compile_options)
  set(_klfengine_interface_tgt_prop_compile_options )
endif()

get_target_property(
  _klfengine_interface_tgt_prop_include_directories
  ${_klfengine_target_name} INTERFACE_INCLUDE_DIRECTORIES
  )
if(NOT _klfengine_interface_tgt_prop_include_directories)
  set(_klfengine_interface_tgt_prop_include_directories )
endif()

get_target_property(
  _klfengine_interface_tgt_prop_link_libraries
  ${_klfengine_target_name} INTERFACE_LINK_LIBRARIES
  )
if(NOT _klfengine_interface_tgt_prop_link_libraries)
  set(_klfengine_interface_tgt_prop_link_libraries )
endif()


#
# nlohmann/json
#
set(_klfengine_interface_tgt_prop_link_libraries
  ${_klfengine_interface_tgt_prop_link_libraries}
  "nlohmann_json::nlohmann_json"
  )

#
# arun11299/subprocess
#
set(_klfengine_interface_tgt_prop_include_directories
  ${_klfengine_interface_tgt_prop_include_directories}
  "${arun11299_subprocess_INCLUDE}"
  )

# #
# # sheredom/subprocess.h
# #
# set(_klfengine_interface_tgt_prop_include_directories
#   ${_klfengine_interface_tgt_prop_include_directories}
#   "${SHEREDOM_SUBPROCESS_INCLUDE}"
# )

#
# mpark/variant
#
if(KLFENGINE_USE_MPARK_VARIANT)
  set(_klfengine_interface_tgt_prop_compile_options
    ${_klfengine_interface_tgt_prop_compile_options}
    "-DKLFENGINE_USE_MPARK_VARIANT"
  )
  set(_klfengine_interface_tgt_prop_link_libraries
    ${_klfengine_interface_tgt_prop_link_libraries}
    "mpark_variant"
  )
endif()

#
# gulrak/filesystem
#
if(KLFENGINE_USE_GULRAK_FILESYSTEM)
  set(_klfengine_interface_tgt_prop_compile_options
    ${_klfengine_interface_tgt_prop_compile_options}
    "-DKLFENGINE_USE_GULRAK_FILESYSTEM"
  )
  set(_klfengine_interface_tgt_prop_link_libraries
    ${_klfengine_interface_tgt_prop_link_libraries}
    "ghcFilesystem::ghc_filesystem"
  )
endif()

#
# ghostscript (headers)
#
if(KLFENGINE_USE_LINKED_GHOSTSCRIPT)
  set(_klfengine_interface_tgt_prop_compile_options
    ${_klfengine_interface_tgt_prop_compile_options}
    "-DKLFENGINE_USE_LINKED_GHOSTSCRIPT"
  )
endif()
if(KLFENGINE_USE_LOAD_GHOSTSCRIPT)
  set(_klfengine_interface_tgt_prop_compile_options
    ${_klfengine_interface_tgt_prop_compile_options}
    "-DKLFENGINE_USE_LOAD_GHOSTSCRIPT"
  )
endif()
if(KLFENGINE_USE_LINKED_GHOSTSCRIPT OR KLFENGINE_USE_LOAD_GHOSTSCRIPT)
  set(_klfengine_interface_tgt_prop_include_directories
    ${_klfengine_interface_tgt_prop_include_directories}
    "${GHOSTSCRIPT_INCLUDE}"
  )
endif()

#
# ghostscript (library)
#
if(KLFENGINE_USE_LINKED_GHOSTSCRIPT)
  set(_klfengine_interface_tgt_prop_link_libraries
    ${_klfengine_interface_tgt_prop_link_libraries}
    "${GHOSTSCRIPT_LIB}"
  )
endif()


#
# Implementation in a separate compilation unit?
#
if(KLFENGINE_SEPARATE_IMPLEMENTATION)
  set(_klfengine_interface_tgt_prop_compile_options
    ${_klfengine_interface_tgt_prop_compile_options}
    "-DKLFENGINE_SEPARATE_IMPLEMENTATION"
  )
endif()


#
# Set appropriate properties
# --------------------------
#

set_target_properties(${_klfengine_target_name}
  PROPERTIES
    INTERFACE_COMPILE_OPTIONS "${_klfengine_interface_tgt_prop_compile_options}"
    INTERFACE_INCLUDE_DIRECTORIES "${_klfengine_interface_tgt_prop_include_directories}"
    INTERFACE_LINK_LIBRARIES "${_klfengine_interface_tgt_prop_link_libraries}"
  )
