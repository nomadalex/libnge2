SET(HEADERS
  nge_app.h
  )

set(SOURCES
  nge_app.cpp
  )

add_our_library(nge_app STATIC ${HEADERS} ${SOURCES})

install(TARGETS nge_app EXPORT nge2 DESTINATION lib)
install(FILES ${HEADERS}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
