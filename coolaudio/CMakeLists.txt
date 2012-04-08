project(coolaudio C CXX)
set(COOLAUDIO_SRCS)
set(COOLAUDIO_LIBS)
set(COOLAUDIO_VERSION 1.1)

if(PSP OR IPHONE)
  list(APPEND COOLAUDIO_SRCS libcoolaudio.c)
endif()

if(ANDROID)
  list(APPEND COOLAUDIO_SRCS
	audio_android.c
	audio_openal.cpp
	audio_openal_wav.c)
  add_package(OPENAL COOLAUDIO_LIBS)
elseif(PSP)
  list(APPEND COOLAUDIO_SRCS
	audio_codec.cpp
	codecs/mp3_codec.cpp
	common/buffered_reader.c
	common/mem64.c
	nge_audio_psp.cpp
	parsers/mp3_parser.cpp
	parsers/oggvorbis_parser.cpp
	parsers/pcm_parser.cpp
	nge_io_mem.c
	)
elseif(IPHONE) # IPHONEOS?
  list(APPEND COOLAUDIO_SRCS
	LSAudioDeleCall.m
	libLSAudio.m
	)
elseif(LINUX)
  list(APPEND COOLAUDIO_SRCS
	nge_audio_linux.c)
  add_package(GSTREAMER COOLAUDIO_LIBS)
endif()

if(PSP)
  list(APPEND COOLAUDIO_LIBS vorbisfile vorbis ogg pspmpeg pspaudiocodec pspaudio)
  ADD_PSPSDK_LIBS(COOLAUDIO_LIBS)
endif()

if(ANDROID)
  add_library(coolaudio STATIC ${COOLAUDIO_SRCS})
elseif(PSP OR LINUX)
  add_library(coolaudio STATIC ${COOLAUDIO_SRCS})
  install(TARGETS coolaudio DESTINATION lib)
elseif(WIN32)
  find_library(COOLAUDIO_LOC NAMES coolaudio libcoolaudio PATHS "${CMAKE_SOURCE_DIR}/Win32Depends" ${NGE2_DEPENDENCIES_DIR} PATH_SUFFIXES lib)
  set(COOLAUDIO_LIBRARY ${COOLAUDIO_LOC} PARENT_SCOPE)
endif()

if(WIN32 OR PSP OR LINUX)
  set(COOLAUDIO_LIBS ${COOLAUDIO_LIBS} PARENT_SCOPE)
endif()

if(ANDROID)
  list(FIND OPENAL_LIBRARY optimized OPENAL_OPTIMIZE_INDEX)
  if(NOT OPENAL_OPTIMIZE_INDEX EQUAL -1)
	math(EXPR OPENAL_OPTIMIZE_INDEX "${OPENAL_OPTIMIZE_INDEX} + 1")
	list(GET OPENAL_LIBRARY ${OPENAL_OPTIMIZE_INDEX} OPENAL_LIBRARY_PATH)
  else()
	set(OPENAL_LIBRARY_PATH ${OPENAL_LIBRARY})
  endif()
  set(ENV{OPENAL_LIBRARY_PATH} ${OPENAL_LIBRARY_PATH})

  include(CopyFile)
  set(COOLAUDIO_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
  function(copy_coolaudio_android_files target android_dir)
	copy_file("LibCoolAudio.java for ${target}"
	  "${COOLAUDIO_DIR}/LibCoolAudio.java"
	  "${android_dir}/src/org/libnge/nge2/LibCoolAudio.java")
	copy_file("openal for ${target}"
	  "$ENV{OPENAL_LIBRARY_PATH}"
	  "${android_dir}/libs/${ARMEABI_NDK_NAME}/libopenal.so")
	add_custom_target(${target}_android_coolaudio
	  DEPENDS "LibCoolAudio.java for ${target}" "openal for ${target}")
	add_dependencies(${target}_android_coolaudio coolaudio)
	add_dependencies(${target} ${target}_android_coolaudio)
  endfunction()
endif()

install(FILES audio_interface.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
