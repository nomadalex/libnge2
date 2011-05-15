project(hge C CXX)

set(HGE_VERSION 1.7)
string(REGEX MATCH "^[0-9]+[.][0-9]+" HGE_SOVERSION ${HGE_VERSION})
set(HGE_DLL_SHORTVER 17)

aux_source_directory(. HGE_SRCS)
set(HGE_LIBS nge2_orig)

# Search for C header files in these directories.
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_our_library(hge STATIC ${HGE_SRCS})
target_link_libraries(hge ${HGE_LIBS})

install(TARGETS hge DESTINATION lib)
install(FILES hgecolor.h hgeparticle.h hgerect.h hgevector.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
