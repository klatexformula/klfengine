# - Try to find arun11299/cpp-subprocess header
#
# Once done this will define
#
#  arun11299_cppsubprocess_FOUND - System has this library
#
#  target arun11299::cppsubprocess - against which you can build projects with
#  target_link_libraries()

find_path(arun11299_cppsubprocess_INCLUDE_DIR subprocess/subprocess.hpp)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set
# arun11299_cppsubprocess_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(
  arun11299_cppsubprocess
  DEFAULT_MSG
  arun11299_cppsubprocess_INCLUDE_DIR)

mark_as_advanced(arun11299_cppsubprocess_INCLUDE_DIR )

if(arun11299_cppsubprocess_FOUND AND NOT TARGET arun11299::cppsubprocess)

  add_library(arun11299::cppsubprocess INTERFACE IMPORTED)

  set_target_properties(arun11299::cppsubprocess PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${arun11299_cppsubprocess_INCLUDE_DIR}"
  )

endif()
