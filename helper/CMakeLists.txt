set(HELPER_SRCS
  CppSQLite3.cpp
  dictionary.c
  iniparser.c
  package.cpp
  )

add_our_library(nge_helper STATIC ${HELPER_SRCS})
add_link_libraries(nge_helper nge2)

install(TARGETS nge_helper EXPORT nge2 DESTINATION lib)
install(FILES CppSQLite3.h dictionary.h iniparser.h package.h  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
