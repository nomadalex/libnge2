# - Try to find SDL mixer
# Once done, this will define
#
#  SDLMIXER_LIBRARIES, the name of the library to link against
#  SDLMIXER_FOUND, if false, do not try to link to SDL mixer
#  SDLMIXER_INCLUDE_DIRS, where to find headers

if(NOT SDLMIXER_FOUND)
  include(PkgUtils)
  findpkg_begin(SDLMIXER)

  define_find_pkg(SDLMIXER
	PKG_CONFIG_NAME SDL_mixer
	LIBRARY_HINTS SDL_mixer
	HEADER_HINTS SDL_mixer.h)

  findpkg_finish(SDLMIXER)
endif()
