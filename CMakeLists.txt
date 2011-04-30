cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Search in the `cmake' directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#-----------------------------------------------------------------------------#
#
# Build options
#

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

# Set the project name.
project(NGE2 C CXX)
SET(NAME NGE2)

include(Common)

set(${NAME}_VERSION 2.1)
string(REGEX MATCH "^[0-9]+[.][0-9]+" ${NAME}_SOVERSION ${${NAME}_VERSION})
set(${NAME}_DLL_SHORTVER 21)

# common src files
set(${NAME}_SRCS
  src/nge_bmp.c
  src/nge_charsets.c
  src/nge_debug_log.c
  src/nge_font.c
  src/nge_font_freetype.c
  src/nge_font_hzk.c
  src/nge_font_nfont.c
  src/nge_gif_animation.c
  src/nge_gif_show.c
  src/nge_image.c
  src/nge_image_effect.c
  src/nge_image_ext.c
  src/nge_input.c
  src/nge_io_dir.c
  src/nge_io_file.c
  src/nge_jpg.c
  src/nge_main.c
  src/nge_misc.c
  src/nge_png.c
  src/nge_rbtree.c
  src/nge_rle.c
  src/nge_sys_compatible.c
  src/nge_tex_cache.c
  src/nge_tga.c
  src/nge_timer.c
  )
set(${NAME}_LIBS)              # always link with these libraries

if(UNIX OR WIN32 OR APPLE)
  list(APPEND ${NAME}_SRCS
	src/nge_graphics.c)
endif()

if(PSP)
  include_directories(${CMAKE_SOURCE_DIR}/include/psp)
  list(APPEND ${NAME}_SRCS
	src/psp/nge_graphics.c
	src/psp/vram.c
	)
endif()

# depends
include_directories(mmgr)
add_subdirectory(mmgr)
option(USE_MMGR "Useful for debug." off)
if(${CMAKE_CONFIGURATION_TYPES} STREQUAL "Debug")
  if(USE_MMGR)
	add_definitions(-DMMGR)
	list(APPEND ${NAME}_LIBS mmgr)
  endif()
endif()

include_directories(coolaudio)
add_subdirectory(coolaudio)
list(APPEND NGE2_LIBS coolaudio)

macro(add_package name include libraries)
  find_package(${name})
  include_directories(${${include}})
  list(APPEND ${NAME}_LIBS ${${libraries}})
  # message("${name} : ${${include}} -- ${${libraries}}")
endmacro()

#-----------------------------------------------------------------------------#
#
# Platform drivers
#

add_package(ZLIB ZLIB_INCLUDE_DIR ZLIB_LIBRARIES)
add_package(JPEG JPEG_INCLUDE_DIR JPEG_LIBRARIES)
add_package(PNG PNG_INCLUDE_DIR PNG_LIBRARIES)
add_package(Freetype FREETYPE_INCLUDE_DIRS FREETYPE_LIBRARIES)
add_package(GIF GIF_INCLUDE_DIR GIF_LIBRARIES)

if(UNIX OR WIN32)
  add_package(SDL SDL_INCLUDE_DIR SDL_LIBRARY)
  add_package(OpenGL OPENGL_INCLUDE_DIR OPENGL_LIBRARIES)
endif()

list(APPEND NGE2_LIBS g c stdc++)

if(PSP)
  list(APPEND NGE2_LIBS vorbisfile vorbis ogg pspmpeg pspaudiocodec pspaudio psppower pspgum pspgu psprtc)
endif()

# -- Unix --
if(UNIX)
  find_package(Threads)
  if(NOT CMAKE_USE_PTHREADS_INIT)
	message(FATAL_ERROR "Unix port requires pthreads support.")
  endif()
  set(_HAVE_LIBPTHREAD 1)
  list(APPEND ${NAME}_LIBS m ${CMAKE_THREAD_LIBS_INIT})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

option(USE_STDOUT_DEBUG "debug logs use stdout." on)
if(${CMAKE_CONFIGURATION_TYPES} STREQUAL "Debug")
  if(USE_STDOUT_DEBUG)
	add_definitions(-D_DEBUG_STDOUT)
  endif()
endif()

list(APPEND ${NAME}_LIBS g c stdc++)

if(PSP)
  ADD_PSPSDK_LIBS(${NAME}_LIBS)
endif()

SET(${NAME}_TYPE STATIC)
add_our_library(nge2 ${${NAME}_TYPE} ${${NAME}_SRCS})
IF(${${NAME}_TYPE} STREQUAL STATIC)
  SET(${NAME}_LIBS ${${NAME}_LIBS} PARENT_SCOPE)
ELSE()
  target_link_libraries(nge2 ${${NAME}_LIBS})
ENDIF()

install(TARGETS nge2 EXPORT nge2 DESTINATION lib)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(EXPORT nge2 DESTINATION share/cmake/Modules)

# addons
# hge particle system
include_directories(hge)
add_subdirectory(hge)

# helpers
include_directories(helper)
add_subdirectory(helper)

if(PSP)
  include_directories(dirent)
  add_subdirectory(dirent)
  include_directories(movie)
  add_subdirectory(movie)
endif()

if(PSP OR WIN32)
  include_directories(net)
  add_subdirectory(net)
endif()

include_directories(app)
add_subdirectory(app)

add_subdirectory(samples)
