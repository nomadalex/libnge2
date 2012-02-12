set(HELPER_SRCS
  CppSQLite3.cpp
  dictionary.c
  iniparser.c
  package.cpp
  )

if(SQLITE3_FOUND)
  include_directories(${SQLITE3_INCLUDE_DIR})
  add_addon_srcs(NGE2_SRCS HELPER_SRCS helper)
  list(APPEND NGE2_LIBS ${SQLITE3_LIBRARIES})
endif()

install(FILES CppSQLite3.h dictionary.h iniparser.h package.h  DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
