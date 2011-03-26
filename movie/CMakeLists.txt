set(MOVIE_SRCS
  nge_movie.c
  )

add_our_library(nge_movie STATIC ${MOVIE_SRCS})
add_link_libraries(nge_movie nge2)

install(TARGETS nge_movie EXPORT nge2 DESTINATION lib)
install(FILES nge_movie.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
