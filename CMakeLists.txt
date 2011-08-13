cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Search in the directorys for additional CMake modules.
set(CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/CMake"
  "${CMAKE_SOURCE_DIR}/CMake/Utils"
  "${CMAKE_SOURCE_DIR}/CMake/Packages"
)
set(CMAKE_TEMPLATE_PATH "${CMAKE_SOURCE_DIR}/CMake/Templates")

if(NOT BUILD_FOR)
  set(BUILD_FOR Normal CACHE STRING
	"Choose the os of build, options are:
     Normal PSP Android."
	 FORCE)
endif()

if(${BUILD_FOR} STREQUAL PSP)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/CMake/Utils/psp.toolchain.cmake")
elseif(${BUILD_FOR} STREQUAL Android)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/CMake/Utils/android.toolchain.cmake")
endif()

# Set the project name.
project(NGE2)
set(NGE2_SRCS)
set(NGE2_LIBS)
set(NGE2_VERSION 2.1)

option(BUILD_SAMPLES "Whether build samples" ON)

if(ANDROID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
endif()

include(Common)
include(SetupDepends)

set(NGE2_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE INITERNAL
  "Real variable for buile." FORCE)

# Put libraries into `lib'.
if(ANDROID)
  set_output_root(${CMAKE_BINARY_DIR})
else()
  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
endif()

# Search for C header files in these directories.
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# common src files
list(APPEND NGE2_SRCS
  src/nge_bmp.c
  src/nge_charsets.c
  src/nge_debug_log.c
  src/nge_font.c
  src/nge_font_freetype.c
  src/nge_font_hzk.c
  src/nge_font_nfont.c
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
  src/nge_graphics.c
  src/nge_app.c
  src/nge_app.cpp
  )
list(APPEND NGE2_LIBS ${NGE2_DEP_LIBS})

if(NOT ANDROID)
  list(APPEND NGE2_SRCS
	src/nge_gif_animation.c
	src/nge_gif_show.c
	)
endif()

if(PSP)
  include_directories(${CMAKE_SOURCE_DIR}/include/psp)
  list(APPEND NGE2_SRCS
	src/psp/nge_graphics.c
	src/psp/vram.c
	)
elseif(ANDROID)
  list(APPEND NGE2_SRCS
	src/android/nge_jni.cpp
	)
endif()

# depends
set(_pack_libs)

if(${NGE2_BUILD_TYPE} STREQUAL "Debug")
  option(USE_MMGR "Useful for debug." off)
  if(USE_MMGR)
	include_directories(mmgr)
	add_subdirectory(mmgr)
	add_definitions(-DMMGR)
	list(APPEND NGE2_LIBS mmgr)
	list(APPEND _pack_libs mmgr)
  endif()
endif()

include_directories(coolaudio)
add_subdirectory(coolaudio)
if(WIN32)
  list(APPEND NGE2_LIBS ${COOLAUDIO_LIBRARY})
else()
  LIST(APPEND NGE2_LIBS coolaudio)
endif()
IF(COOLAUDIO_LIBS)
  LIST(APPEND NGE2_LIBS ${COOLAUDIO_LIBS})
ENDIF()

if(COMPILER_GCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
endif()

if(${NGE2_BUILD_TYPE} STREQUAL "Debug")
  option(USE_STDOUT_DEBUG "debug logs use stdout." on)
  if(USE_STDOUT_DEBUG)
	add_definitions(-D_DEBUG_STDOUT)
  endif()
endif()

if(PSP)
  list(APPEND NGE2_LIBS psppower pspgum pspgu psprtc)
endif()

if(WIN32)
  list(APPEND NGE2_LIBS ${SDLMAIN_LIBRARY})
elseif(ANDROID)
  list(APPEND NGE2_LIBS log GLESv1_CM)
else()
  list(APPEND NGE2_LIBS g c stdc++)
endif()

if(PSP)
  add_pspsdk_libs(NGE2_LIBS)
endif()

set(NGE2_ORIG_LIBS ${NGE2_LIBS})
add_our_library(nge2_orig STATIC ${NGE2_SRCS})

# addons
macro(add_addon subdir target)
  include_directories(${subdir})
  add_subdirectory(${subdir})
  list(APPEND _pack_libs ${target})
endmacro()

# hge particle system
add_addon(hge hge)

# helpers
add_addon(helper nge_helper)

if(PSP)
  add_addon(dirent nge_dirent)
  add_addon(movie nge_movie)
endif()

if(PSP)
  add_addon(net nge_net)
endif()

include(LibUtils)
merge_static_libs(nge2 LIBRARIES ${_pack_libs} nge2_orig)
install(TARGETS nge2 DESTINATION lib)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
