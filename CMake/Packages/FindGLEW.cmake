# - Try to find GLEW
# Once done, this will define
#
#  GLEW_FOUND - system has glew lib
#  GLEW_INCLUDE_DIR - the glew lib include directories
#  GLEW_LIBRARY - link these to use glew

if(NOT GLEW_FOUND)
  include(PkgUtils)
  findpkg_begin(GLEW)

  define_find_pkg(GLEW
	LIBRARY_HINTS glew glew32 libglew libglew32
	HEADER_HINTS gl/glew.h)

  findpkg_finish(GLEW)
endif()
