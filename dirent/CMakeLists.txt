set(DIRENT_SRCS
  directory.c
  fat.c
  miniconv.c
  nge_dirent.cpp
  )

add_addon_srcs(NGE2_SRCS DIRENT_SRCS dirent)

install(FILES dirent/nge_dirent.hpp dirent/nge_ch_io_dir.h  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
