set(DIRENT_SRCS
  directory.c
  fat.c
  miniconv.c
  nge_dirent.cpp
  )

add_our_library(nge_dirent STATIC ${DIRENT_SRCS})
target_link_libraries(nge_dirent nge2_orig)

install(TARGETS nge_dirent DESTINATION lib)
install(FILES nge_dirent.hpp nge_ch_io_dir.h  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
