# - Try to find unzip
# Once done, this will define
#
#  UNZIP_INCLUDE_DIRS, where to find png.h, etc.
#  UNZIP_LIBRARIES, the libraries to link against to use UNZIP.
#  UNZIP_FOUND, If false, do not try to use UNZIP.

if(NOT UNZIP_FOUND)
  include(PkgUtils)
  findpkg_begin(OPENAL)

  define_find_pkg(OPENAL
	LIBRARY_HINTS libopenal openal al OpenAL OpenAL32
	HEADER_HINTS al.h AL/al.h OpenAL/al.h)

  findpkg_finish(OPENAL)
endif()
