# - Try to find FreeType
# Once done, this will define
#
#  FREETYPE_FOUND - system has FreeType
#  FREETYPE_INCLUDE_DIRS - the FreeType include directories 
#  FREETYPE_LIBRARY - link these to use FreeType

if(NOT FREETYPE_FOUND)
  include(PkgUtils)
  findpkg_begin(FREETYPE)

  # prefer static library over framework 
  set(CMAKE_FIND_FRAMEWORK "LAST")

  define_find_pkg(FREETYPE
	PKG_CONFIG_NAME freetype2
	LIBRARY_HINTS freetype2311 freetype239 freetype238 freetype235 freetype219 freetype
	HEADER_HINTS freetype/freetype.h
	HEADER_SUFFIXES freetype2)

  find_path(FREETYPE_FT2BUILD_INCLUDE_DIR NAMES ft2build.h HINTS ${FREETYPE_INC_SEARCH_PATH} ${FREETYPE_PKGC_INCLUDE_DIRS})

  # Reset framework finding
  set(CMAKE_FIND_FRAMEWORK "FIRST")

  findpkg_finish(FREETYPE)
  mark_as_advanced(FREETYPE_FT2BUILD_INCLUDE_DIR)
  if (NOT FREETYPE_FT2BUILD_INCLUDE_DIR STREQUAL FREETYPE_INCLUDE_DIR)
	list(APPEND FREETYPE_INCLUDE_DIRS ${FREETYPE_FT2BUILD_INCLUDE_DIR})
  endif ()
endif()
