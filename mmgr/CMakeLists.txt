set(MMGR_SRCS
  mmgr.cpp
  )

add_our_library(mmgr STATIC ${MMGR_SRCS})

install(TARGETS mmgr EXPORT nge2 DESTINATION lib)
install(FILES mmgr.h nommgr.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
