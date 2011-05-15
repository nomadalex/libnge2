# - Try to find SDL
# Once done, this will define
# 
# SDL_LIBRARIES, the name of the library to link against
# SDL_FOUND, if false, do not try to link to SDL
# SDL_INCLUDE_DIRS, where to find SDL.h
#
# This module responds to the the flag:
# SDL_BUILDING_LIBRARY
# If this is defined, then no SDL_main will be linked in because
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the the proper link flags
# as part of the returned SDL_LIBRARY variable.

if(NOT SDL_FOUND)
  include(PkgUtils)
  findpkg_begin(SDL)

  # SDL may require threads on your system.
  # The Apple build may not need an explicit flag because one of the
  # frameworks may already provide it.
  # But for non-OSX systems, I will use the CMake Threads package.
  IF(NOT APPLE)
	FIND_PACKAGE(Threads)
  ENDIF(NOT APPLE)

  define_find_pkg(SDL
	PKG_CONFIG_NAME sdl
	LIBRARY_HINTS SDL SDL-1.1
	HEADER_HINTS SDL.h
	HEADER_SUFFIXES SDL)

  findpkg_finish(SDL)

  # For SDLmain
  IF(NOT SDL_BUILDING_LIBRARY)
	find_library(SDLMAIN_LIBRARY NAMES SDLmain SDLmain-1.1 HINTS ${SDL_LIB_SEARCH_PATH} ${SDL_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "")
	IF(SDLMAIN_LIBRARY)
      SET(SDL_LIBRARY_TEMP ${SDLMAIN_LIBRARY} ${SDL_LIBRARY_TEMP})
	ENDIF(SDLMAIN_LIBRARY)
  ENDIF(NOT SDL_BUILDING_LIBRARY)

  # MinGW needs an additional library, mwindows
  # It's total link flags should look like -lmingw32 -lSDLmain -lSDL -lmwindows
  # (Actually on second look, I think it only needs one of the m* libraries.)
  IF(MINGW)
	list(APPEND SDL_LIBRARIES mingw32)
  ENDIF(MINGW)

  # For OS X, SDL uses Cocoa as a backend so it must link to Cocoa.
  # CMake doesn't display the -framework Cocoa string in the UI even
  # though it actually is there if I modify a pre-used variable.
  # I think it has something to do with the CACHE STRING.
  # So I use a temporary variable until the end so I can set the
  # "real" variable in one-shot.
  IF(APPLE)
	list(APPEND SDL_LIBRARIES "-framework Cocoa")
  ENDIF(APPLE)

  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if I used the Threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  IF(NOT APPLE)
	list(APPEND SDL_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  ENDIF(NOT APPLE)
endif()
