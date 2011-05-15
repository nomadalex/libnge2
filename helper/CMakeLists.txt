set(HELPER_SRCS
  CppSQLite3.cpp
  dictionary.c
  iniparser.c
  package.cpp
  )

add_our_library(nge_helper STATIC ${HELPER_SRCS})

install(TARGETS nge_helper DESTINATION lib)
install(FILES CppSQLite3.h dictionary.h iniparser.h package.h  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
