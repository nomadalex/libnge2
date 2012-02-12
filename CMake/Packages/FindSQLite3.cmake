# - Try to find sqlite3
# Once done, this will define
#
#  SQLITE3_INCLUDE_DIRS, where to find png.h, etc.
#  SQLITE3_LIBRARIES, the libraries to link against to use SQLITE3.
#  SQLITE3_FOUND, If false, do not try to use SQLITE3.

if(NOT SQLITE3_FOUND)
  include(PkgUtils)
  findpkg_begin(SQLITE3)

  define_find_pkg(SQLITE3
	PKG_CONFIG_NAME sqlite3
	LIBRARY_HINTS libsqlite3 libsqlite sqlite3
	HEADER_HINTS sqlite3.h)

  findpkg_finish(SQLITE3)
endif()
