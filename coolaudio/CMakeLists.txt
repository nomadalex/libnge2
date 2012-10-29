project(coolaudio C CXX)
set(COOLAUDIO_SRCS)
set(COOLAUDIO_LIBS)
set(COOLAUDIO_VERSION 1.1)

include_directories(. ../and_jni_base ../include)

if(PSP OR IPHONE)
  list(APPEND COOLAUDIO_SRCS libcoolaudio.c)
endif()

if(ANDROID)
  list(APPEND COOLAUDIO_SRCS
	android_rawaudio.c
	android_wav.c
	android_mediaplayer.c
	audio_android.c)
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
  include(CopyFile)
  set(COOLAUDIO_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
  function(copy_coolaudio_android_files target android_dir)
	copy_file("LibCoolAudio.java for ${target}"
	  "${COOLAUDIO_DIR}/LibCoolAudio.java"
	  "${android_dir}/src/org/libnge/nge2/LibCoolAudio.java")
	copy_file("RawAudio.java for ${target}"
	  "${COOLAUDIO_DIR}/RawAudio.java"
	  "${android_dir}/src/org/libnge/nge2/RawAudio.java")
	add_custom_target(${target}_android_coolaudio
	  DEPENDS
	  "LibCoolAudio.java for ${target}"
	  "RawAudio.java for ${target}")
	add_dependencies(${target}_android_coolaudio coolaudio)
	add_dependencies(${target} ${target}_android_coolaudio)
  endfunction()
endif()
