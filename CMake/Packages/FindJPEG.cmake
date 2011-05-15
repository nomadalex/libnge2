# - Try to find JPEG
# Once done, this will define
#
#  JPEG_FOUND - system has JPEGlib
#  JPEG_INCLUDE_DIR - the JPEGlib include directories 
#  JPEG_LIBRARY - link these to use JPGElib

if(NOT JPEG_FOUND)
  include(PkgUtils)
  findpkg_begin(JPEG)

  define_find_pkg(JPEG
	LIBRARY_HINTS jpeg
	HEADER_HINTS jpeglib.h)

  findpkg_finish(JPEG)
endif()
