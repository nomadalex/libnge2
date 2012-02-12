set(MOVIE_SRCS
  nge_movie.c
  )

add_addon_srcs(NGE2_SRCS MOVIE_SRCS movie)

install(FILES nge_movie.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
