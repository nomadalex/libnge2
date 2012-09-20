set(HELPER_SRCS
  CppSQLite3.cpp
  dictionary.c
  iniparser.c
  package.cpp
  )

if(SQLITE3_FOUND)
  add_addon_srcs(helper
	HAS_SUBDIR_IN_INCLUDE
	SRCS ${HELPER_SRCS}
	LIBS ${SQLITE3_LIBRARIES}
	INCLUDES ${SQLITE3_INCLUDE_DIR})
endif()
