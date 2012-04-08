# - Try to find openal
# Once done, this will define
#
#  OPENAL_INCLUDE_DIRS, where to find headers, etc.
#  OPENAL_LIBRARIES, the libraries to link against for use OpenAL.
#  OPENAL_FOUND, If false, do not try to use OpenAL.

if(NOT OPENAL_FOUND)
  include(PkgUtils)
  findpkg_begin(OPENAL)

  define_find_pkg(OPENAL
	LIBRARY_HINTS libopenal openal al OpenAL OpenAL32
	HEADER_HINTS al.h AL/al.h OpenAL/al.h)

  findpkg_finish(OPENAL)
endif()
