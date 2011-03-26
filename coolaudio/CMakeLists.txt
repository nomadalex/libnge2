project(coolaudio C CXX)

set(COOLAUDIO_VERSION 1.1)
string(REGEX MATCH "^[0-9]+[.][0-9]+" COOLAUDIO_SOVERSION ${COOLAUDIO_VERSION})
set(COOLAUDIO_DLL_SHORTVER 11)

set(COOLAUDIO_SRCS
  libcoolaudio.c
)
set(COOLAUDIO_LIBS)

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

# Search for C header files in these directories.
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX OR WIN32)
  find_package(SDL_mixer)
  include_directories(${SDLMIXER_INCLUDE_DIR})
  list(APPEND COOLAUDIO_LIBS ${SDLMIXER_LIBRARY})
endif()

add_our_library(coolaudio STATIC ${COOLAUDIO_SRCS})
target_link_libraries(coolaudio ${COOLAUDIO_LIBS})

install(TARGETS coolaudio EXPORT nge2 DESTINATION lib)
install(FILES audio_interface.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)