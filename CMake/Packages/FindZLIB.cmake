# - Try to find ZLIB
# Once done, this will define
#
#  ZLIB_FOUND - system has ZLIB
#  ZLIB_INCLUDE_DIRS - the ZLIB include directories 
#  ZLIB_LIBRARIES - link these to use ZLIB

if(NOT ZLIB_FOUND)
  include(PkgUtils)
  findpkg_begin(ZLIB)

  define_find_pkg(ZLIB
	PKG_CONFIG_NAME zzip-zlib-config
	LIBRARY_HINTS z zlib zdll
	HEADER_HINTS zlib.h)

  findpkg_finish(ZLIB)
endif()
