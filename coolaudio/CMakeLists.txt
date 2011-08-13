project(coolaudio C CXX)
set(COOLAUDIO_SRCS)
set(COOLAUDIO_LIBS)
set(COOLAUDIO_VERSION 1.1)

list(APPEND COOLAUDIO_LIBS ${COOLAUDIO_DEP_LIBS})

if(PSP OR IPHONE)
  list(APPEND COOLAUDIO_SRCS libcoolaudio.c)
endif()

if(ANDROID)
  include(CopyFile)
  set(COOLAUDIO_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
  function(copy_coolaudio_android_java_files target dir)
	set(fromfile "${COOLAUDIO_DIR}/LibCoolAudio.java")
	set(tofile "${dir}/src/org/libnge/nge2/LibCoolAudio.java")
	copy_file("LibCoolAudio.java for ${nm}" ${fromfile} ${tofile})
	add_custom_target(${target}_audio_java DEPENDS "LibCoolAudio.java for ${nm}")
	add_dependencies(${target} ${target}_audio_java)
  endfunction()
  list(APPEND COOLAUDIO_SRCS
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
elseif(IPHONE) # IPHONESO?
  list(APPEND COOLAUDIO_SRCS
	LSAudioDeleCall.m
	libLSAudio.m
	)
elseif(LINUX)
  list(APPEND COOLAUDIO_SRCS
	nge_audio_linux.c)
endif()

if(PSP)
  list(APPEND COOLAUDIO_LIBS vorbisfile vorbis ogg pspmpeg pspaudiocodec pspaudio)
  ADD_PSPSDK_LIBS(COOLAUDIO_LIBS)
endif()

if(PSP OR LINUX)
  add_our_library(coolaudio STATIC ${COOLAUDIO_SRCS})
  install(TARGETS coolaudio DESTINATION lib)
elseif(WIN32)
  find_library(COOLAUDIO_LOC NAMES coolaudio libcoolaudio PATHS "${CMAKE_SOURCE_DIR}/Win32Depends" ${NGE2_DEPENDENCIES_DIR} PATH_SUFFIXES lib)
  set(COOLAUDIO_LIBRARY ${COOLAUDIO_LOC} PARENT_SCOPE)
  set(COOLAUDIO_LIBS ${COOLAUDIO_LIBS} PARENT_SCOPE)
endif()

install(FILES audio_interface.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
