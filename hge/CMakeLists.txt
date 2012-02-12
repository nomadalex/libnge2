set(HGE_VERSION 1.7)
string(REGEX MATCH "^[0-9]+[.][0-9]+" HGE_SOVERSION ${HGE_VERSION})
set(HGE_DLL_SHORTVER 17)

aux_source_directory(hge HGE_SRCS)
list(APPEND NGE2_SRCS ${HGE_SRCS})

install(FILES hgecolor.h hgeparticle.h hgerect.h hgevector.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
