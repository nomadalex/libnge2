# - Try to find unzip
# Once done, this will define
#
#  UNZIP_INCLUDE_DIRS, where to find png.h, etc.
#  UNZIP_LIBRARIES, the libraries to link against to use UNZIP.
#  UNZIP_FOUND, If false, do not try to use UNZIP.

if(NOT UNZIP_FOUND)
  include(PkgUtils)
  findpkg_begin(UNZIP)

  define_find_pkg(UNZIP
	LIBRARY_HINTS libunzip unzip
	HEADER_HINTS unzip.h)

  findpkg_finish(UNZIP)
endif()
