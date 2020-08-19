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

set(KLFENGINE_VERSION
  "${KLFENGINE_VERSION_MAJOR}.${KLFENGINE_VERSION_MINOR}.${KLFENGINE_VERSION_RELEASE}${KLFENGINE_VERSION_SUFFIX}")
