set(MMGR_SRCS
  mmgr.cpp
  )

add_addon_srcs(NGE2_SRCS MMGR_SRCS mmgr)

install(FILES mmgr.h nommgr.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
