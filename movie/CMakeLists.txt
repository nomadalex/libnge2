set(MOVIE_SRCS
  nge_movie.c
  )

add_addon_srcs(movie
  HAS_SUBDIR_IN_INCLUDE
  SRCS ${MOVIE_SRCS})
