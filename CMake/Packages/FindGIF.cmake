# - Try to find GIF
# Once done, this will define
#
#  GIF_INCLUDE_DIR, where to find gif_lib.h, etc.
#  GIF_LIBRARIES, the libraries to link against to use giflib.
#  GIF_FOUND, If false, do not try to use giflib.

if(NOT GIF_FOUND)
  include(PkgUtils)
  findpkg_begin(GIF)

  define_find_pkg(GIF
	LIBRARY_HINTS gif libgif ungif libungif giflib
	HEADER_HINTS gif_lib.h)

  findpkg_finish(GIF)
endif()
