#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_PATH
# GLEW_LIBRARY
# 

if(NOT GLEW_FOUND)
  include(PkgUtils)
  findpkg_begin(GLEW)

  define_find_pkg(GLEW
	PKG_CONFIG_NAME glew
	LIBRARY_HINTS glew GLEW glew32 glew32s libglew libglew32
	HEADER_HINTS gl/glew.h GL/glew.h)
	
  findpkg_finish(GLEW)
endif()