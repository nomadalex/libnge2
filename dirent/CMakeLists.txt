set(DIRENT_SRCS
  directory.c
  fat.c
  miniconv.c
  nge_dirent.cpp
  )

add_addon_srcs(dirent
  INCLUDE_THIS_DIR
  SRCS ${DIRENT_SRCS})
