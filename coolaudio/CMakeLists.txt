project(coolaudio C CXX)

SET(NAME COOLAUDIO)

set(${NAME}_VERSION 1.1)
string(REGEX MATCH "^[0-9]+[.][0-9]+" ${NAME}_SOVERSION ${${NAME}_VERSION})
set(${NAME}_DLL_SHORTVER 11)

set(${NAME}_SRCS
  libcoolaudio.c
)
set(${NAME}_LIBS)

if(WIN32 OR UNIX)
  list(APPEND ${NAME}_SRCS
	nge_audio.cpp)
endif()

if(PSP)
  list(APPEND ${NAME}_SRCS
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
  list(APPEND ${NAME}_LIBS vorbisfile vorbis ogg pspmpeg pspaudiocodec pspaudio)
  ADD_PSPSDK_LIBS(${NAME}_LIBS)
endif()

if(APPLE) # IPHONESO?
  list(APPEND ${NAME}_SRCS
	LSAudioDeleCall.m
	libLSAudio.m
	)
endif()

# Search for C header files in these directories.
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX OR WIN32)
  find_package(SDL_mixer)
  include_directories(${SDLMIXER_INCLUDE_DIR})
  list(APPEND ${NAME}_LIBS ${SDLMIXER_LIBRARY})
endif()

SET(${NAME}_TYPE STATIC)
add_our_library(coolaudio ${${NAME}_TYPE} ${${NAME}_SRCS})
IF(${${NAME}_TYPE} STREQUAL STATIC)
  SET(${NAME}_LIBS ${${NAME}_LIBS} PARENT_SCOPE)
ELSE()
  target_link_libraries(coolaudio ${${NAME}_LIBS})
ENDIF()

install(TARGETS coolaudio EXPORT nge2 DESTINATION lib)
install(FILES audio_interface.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
