project(coolaudio C CXX)
set(COOLAUDIO_SRCS)
set(COOLAUDIO_LIBS)
set(COOLAUDIO_VERSION 1.1)

list(APPEND COOLAUDIO_SRCS libcoolaudio.c)
list(APPEND COOLAUDIO_LIBS ${COOLAUDIO_DEP_LIBS})

if(WIN32 OR UNIX)
  list(APPEND COOLAUDIO_SRCS
	nge_audio.cpp)
endif()

if(PSP)
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
endif()

if(APPLE) # IPHONESO?
  list(APPEND COOLAUDIO_SRCS
	LSAudioDeleCall.m
	libLSAudio.m
	)
endif()

if(PSP)
  list(APPEND COOLAUDIO_LIBS vorbisfile vorbis ogg pspmpeg pspaudiocodec pspaudio)
  ADD_PSPSDK_LIBS(COOLAUDIO_LIBS)
endif()

add_our_library(coolaudio STATIC ${COOLAUDIO_SRCS})

install(TARGETS coolaudio EXPORT nge2 DESTINATION lib)
install(FILES audio_interface.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
