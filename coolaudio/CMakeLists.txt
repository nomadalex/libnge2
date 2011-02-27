# Set the project name.
project(coolaudio C CXX)

set(coolaudio_VERSION 2.1)
string(REGEX MATCH "^[0-9]+[.][0-9]+" coolaudio_SOVERSION ${coolaudio_VERSION})
set(coolaudio_DLL_SHORTVER 21)

set(SRCS
  libcoolaudio.c
  nge_audio.cpp
)
set(LIBS)              # always link with these libraries

set(SRCS_PSP
  audio_codec.cpp
  codecs/mp3_codec.cpp
  common/buffered_reader.c
  common/mem64.c
  nge_audio_psp.cpp
  parsers/mp3_parser.cpp
  parsers/oggvorbis_parser.cpp
  parsers/pcm_parser.cpp
)

# Search for C header files in these directories.
include_directories(${PROJECT_SOURCE_DIR})
# include_directories(${PROJECT_BINARY_DIR}/include)

# message("S is ${PROJECT_SOURCE_DIR}")
# message("B is ${PROJECT_BINARY_DIR}")

# Put libraries into `lib'.
# set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(PLAT_LINUX OR PLAT_WIN)
  find_package(SDL_mixer)
  include_directories(${SDLMIXER_INCLUDE_DIR})
  list(APPEND LIBS ${SDLMIXER_LIBRARY})
elseif(PLAT_PSP)
  add_definitions(-DCOOLAUDIO_PSP)
  list(APPEND SRCS ${SRCS_PSP})
endif()

add_our_library(coolaudio STATIC
  ${SRCS})
target_link_libraries(coolaudio ${LIBS})
