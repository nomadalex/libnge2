# - Try to find PNG
# Once done, this will define
#
#  PNG_INCLUDE_DIRS, where to find png.h, etc.
#  PNG_LIBRARIES, the libraries to link against to use PNG.
#  PNG_FOUND, If false, do not try to use PNG.
# None of the above will be defined unles zlib can be found.
# PNG depends on Zlib

if(NOT PNG_FOUND)
  include(PkgUtils)
  findpkg_begin(PNG)

  find_package(ZLIB)
  if(NOT ZLIB_FOUND)
	message(SEND_ERROR "PNG depends on Zlib")
	return()
  endif()

  define_find_pkg(PNG 
	PKG_CONFIG_NAME libpng
	LIBRARY_HINTS png libpng png15 libpng15 png14 libpng14 png12 libpng12
	HEADER_HINTS png.h)

  findpkg_finish(PNG)

  list(APPEND PNG_LIBRARIES ${ZLIB_LIBRARY})
endif()
