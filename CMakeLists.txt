cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Search in the `cmake' directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Psptools)

#-----------------------------------------------------------------------------#
#
# Build options
#

# Set the project name.
project(NGE2 C CXX)

set(NGE2_VERSION 2.1)
string(REGEX MATCH "^[0-9]+[.][0-9]+" NGE2_SOVERSION ${NGE2_VERSION})
set(NGE2_DLL_SHORTVER 21)

set(CMAKE_INSTALL_PREFIX ${PSPSDK_PATH})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are:
	None Debug Release RelWithDebInfo MinSizeRel Profile."
      FORCE)
endif()
# Restrict configuration types to the selected build type.
# Note: This needs to be done before the project command
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE INTERNAL "internal")

# Search for C header files in these directories.
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# Put libraries into `lib'.
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

include(Common)

set(SRCS_PSP
  src/psp/directory/directory.c
  src/psp/directory/fat.c
  src/psp/directory/miniconv.c

  src/psp/nge_graphics.c
  src/psp/vram.c

  src/hge/hgecolor.cpp
  src/hge/hgeparticle.cpp
  src/hge/hgepmanager.cpp
  src/hge/hgerect.cpp
  src/hge/hgevector.cpp
  )

set(SRCS
  src/effect/nge_image_effect.c
  src/effect/nge_image_effect.h

  src/helper/CppSQLite3.cpp
  src/helper/dictionary.c
  src/helper/iniparser.c
  src/helper/package.cpp

  src/libnge2.c
  src/movieplay/nge_movie.c

  src/net/event.c
  src/net/log.c
  src/net/nge_net_common.c
  src/net/select.c

  src/nge_charsets.c
  src/nge_ch_io_dir.cpp
  src/nge_debug_log.c
  src/nge_font.c
  src/nge_font_freetype.c
  src/nge_font_hzk.c
  src/nge_font_nfont.c
  src/nge_gif_animation.c
  src/nge_gif_show.c
  src/nge_graphics_ext.c
  src/nge_image_load.c
  src/nge_input_proc.c
  src/nge_io_dir.c
  src/nge_io_file.c
  src/nge_main.c
  src/nge_misc.c
  src/nge_rbtree.c
  src/nge_rle.c
  src/nge_sys_compatible.c
  src/nge_tex_cache.c
  src/nge_timer.c
)
set(LIBS)              # always link with these libraries


#-----------------------------------------------------------------------------#
#
# Platform drivers
#

include_directories(${CMAKE_SOURCE_DIR}/coolaudio)
add_subdirectory(coolaudio)
list(APPEND LIBS coolaudio)

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})
list(APPEND LIBS ${ZLIB_LIBRARIES})

find_package(JPEG)
include_directories(SYSTEM ${JPEG_INCLUDE_DIR})
list(APPEND LIBS ${JPEG_LIBRARIES})

find_package(PNG)
include_directories(SYSTEM ${PNG_INCLUDE_DIR})
list(APPEND LIBS ${PNG_LIBRARIES})

find_package(SDL)
include_directories(SYSTEM ${SDL_INCLUDE_DIR})
list(APPEND LIBS ${SDL_LIBRARIES})

find_package(Freetype)
include_directories(${FREETYPE_INCLUDE_DIRS})
list(APPEND LIBS ${FREETYPE_LIBRARIES})

if(PLAT_WIN OR PLAT_LINUX)
  option(USE_MMGR "Useful for debug." off)
  if(USE_MMGR)
    list(APPEND SRCS
      src/debug/mmgr.cpp
      src/debug/mmgr.h
      src/debug/nommgr.h
      )
  endif()
endif()

# -- Unix --
if(PLAT_UNIX)
  find_package(Threads)
  if(NOT CMAKE_USE_PTHREADS_INIT)
    message(FATAL_ERROR "Unix port requires pthreads support.")
  endif()
  set(_HAVE_LIBPTHREAD 1)
  list(APPEND LIBS m ${CMAKE_THREAD_LIBS_INIT})
  # list(APPEND SRCS ${SRCS_UNIX})

# -- Windows --
elseif(PLAT_WIN)
  list(APPEND SRCS ${SRCS_WIN})

# fixme: I don't have it, so I don't know what it need.
# -- Mac OS X --
# elseif(PLAT_MAC)
#   list(APPEND SRCS ${SRCS_MAC})

# -- PSP --
elseif(PLAT_PSP)
  list(APPEND SRCS ${SRCS_PSP})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
add_definitions(-DNGE2_NET -D_DEBUG_LOG -D_DEBUG_STDOUT)
add_library(nge2 STATIC
  ${SRCS}
  )
target_link_libraries(nge2 ${LIBS})

install_our_library(nge2)

add_subdirectory(samples)
