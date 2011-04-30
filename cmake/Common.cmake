#-----------------------------------------------------------------------------#
#
# Build options
#

option(SHARED "Build shared library." on)
set(BUILD_SHARED_LIBS ${SHARED})    # actual CMake variable

# On some 64-bit platforms, libraries should be installed into `lib64'
# instead of `lib'.  Set this to 64 to do that.
set(LIB_SUFFIX "" CACHE STRING "Suffix for 'lib' directories, e.g. '64'")

option(MAGIC_MAIN "Enable magic main (Unix)" off)

option(WANT_FRAMEWORKS "Want frameworks on Mac OS X" off)
option(WANT_EMBED
  "Make frameworks embeddable in application bundles (Mac OS X)" on)

set(FRAMEWORK_INSTALL_PREFIX "/Library/Frameworks" CACHE STRING
  "Directory in which to install Mac OS X frameworks")

#-----------------------------------------------------------------------------#
#
# Unix platform checks
#

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(FindPkgConfig)
include(TestBigEndian)

if(UNIX)
  # test_big_endian(_BIG_ENDIAN)
  # if(NOT _BIG_ENDIAN)
  # 	set(_LITTLE_ENDIAN 1)
  # endif(NOT _BIG_ENDIAN)

  # check_include_files(dirent.h _HAVE_DIRENT_H)
  # check_include_files(inttypes.h _HAVE_INTTYPES_H)

  # check_include_files(linux/awe_voice.h _HAVE_LINUX_AWE_VOICE_H)
  # check_include_files(linux/input.h _HAVE_LINUX_INPUT_H)
  # # On some systems including linux/joystick.h without sys/types.h results
  # # in conflicting definitions of fd_set.
  # check_include_files("sys/types.h;linux/joystick.h" _HAVE_LINUX_JOYSTICK_H)
  # check_include_files(linux/soundcard.h _HAVE_LINUX_SOUNDCARD_H)
  # check_include_files(machine/soundcard.h _HAVE_MACHINE_SOUNDCARD_H)
  # check_include_files(soundcard.h _HAVE_SOUNDCARD_H)
  # check_include_files(stdint.h _HAVE_STDINT_H)
  # check_include_files(sys/io.h _HAVE_SYS_IO_H)
  # check_include_files(sys/stat.h _HAVE_SYS_STAT_H)
  # check_include_files(sys/time.h _HAVE_SYS_TIME_H)
  # check_include_files(sys/soundcard.h _HAVE_SYS_SOUNDCARD_H)
  # check_include_files(sys/utsname.h _HAVE_SYS_UTSNAME_H)

  # check_function_exists(getexecname _HAVE_GETEXECNAME)
  # check_function_exists(memcmp _HAVE_MEMCMP)
  # check_function_exists(mkstemp _HAVE_MKSTEMP)
  # check_function_exists(mmap _HAVE_MMAP)
  # check_function_exists(mprotect _HAVE_MPROTECT)
  # check_function_exists(sched_yield _HAVE_SCHED_YIELD)
  # check_function_exists(stricmp _HAVE_STRICMP)
  # check_function_exists(strlwr _HAVE_STRLWR)
  # check_function_exists(strupr _HAVE_STRUPR)
  # check_function_exists(sysconf _HAVE_SYSCONF)

  # check_c_source_compiles("
  # 	#include <sys/procfs.h>
  # 	#include <sys/ioctl.h>
  # 	int main(void) {
  # 		struct prpsinfo psinfo;
  # 		ioctl(0, PIOCPSINFO, &psinfo);
  # 		return 0;
  # 	}"
  # 	_HAVE_SV_PROCFS_H
  # 	)
  # check_c_source_compiles("
  # 	#include <sys/procfs.h>
  # 	int main(void) {
  # 		struct prpsinfo psinfo;
  # 		psinfo.pr_argc = 0;
  # 		return 0;
  # 	}"
  # 	_HAVE_PROCFS_ARGCV
  # 	)

  # check_c_source_compiles("
  # 	#include <unistd.h>
  # 	#include <sys/mman.h>
  # 	int main(void) {
  # 		void *x = MAP_FAILED;
  # 	}"
  # 	MAP_FAILED_DEFINED)
  # if(NOT MAP_FAILED_DEFINED)
  # 	set(MAP_FAILED "((void *) -1)")
  # endif()

  # check_c_source_runs("
  # 	static int notsupported = 1;
  # 	void test_ctor (void) __attribute__((constructor));
  # 	void test_ctor (void) { notsupported = 0; }
  # 	int main(void) { return (notsupported); }
  # 	"
  # 	_USE_CONSTRUCTOR)

  # if(MAGIC_MAIN)
  # 	set(_WITH_MAGIC_MAIN 1)
  # endif(MAGIC_MAIN)

  # XXX const
  # XXX inline
  # XXX size_t
endif(UNIX)

#-----------------------------------------------------------------------------#
#
# Compiler and platform setup
#

option(STRICT_WARN "Halt at warnings" off)

set(INSTALL_PREFIX "")

if(CMAKE_COMPILER_IS_GNUCC)
  set(COMPILER_GCC 1)
  set(WFLAGS "-W -Wall -Wno-unused-parameter")
  set(WFLAGS_C_ONLY "-Wdeclaration-after-statement")
  if(STRICT_WARN)
	set(WFLAGS "${WFLAGS} -Werror -Wpointer-arith")
	set(WFLAGS_C_ONLY "${WFLAGS_C_ONLY} -Wmissing-declarations")
	set(WFLAGS_C_ONLY "${WFLAGS_C_ONLY} -Wstrict-prototypes")
  endif(STRICT_WARN)
endif(CMAKE_COMPILER_IS_GNUCC)

if(MINGW)
  set(COMPILER_MINGW 1)
  # Guess MINGDIR from the value of CMAKE_C_COMPILER if it's not set.
  if("$ENV{MINGDIR}" STREQUAL "")
	string(REGEX REPLACE "/bin/[^/]*$" "" MINGDIR "${CMAKE_C_COMPILER}")
	message(STATUS "Guessed MinGW directory: ${MINGDIR}")
  else("$ENV{MINGDIR}" STREQUAL "")
	file(TO_CMAKE_PATH "$ENV{MINGDIR}" MINGDIR)
	message(STATUS "Using MINGDIR: ${MINGDIR}")
  endif("$ENV{MINGDIR}" STREQUAL "")

  # Search in MINGDIR for headers and libraries.
  set(CMAKE_PREFIX_PATH "${MINGDIR}")

  # Install to MINGDIR
  if(INSTALL_PREFIX STREQUAL "")
	set(CMAKE_INSTALL_PREFIX ${MINGDIR})
  else(INSTALL_PREFIX STREQUAL "")
	set(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
  endif(INSTALL_PREFIX STREQUAL "")

  message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endif(MINGW)

if(MSVC)
  set(COMPILER_MSVC 1)
  set(WFLAGS "/W3 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
endif(MSVC)

if(WIN32)
  set(PLAT_WIN 1)
endif()

if(APPLE)
  set(PLAT_MAC 1)

  # Hack to deal with Mac OS X 10.6.  NSQuickDrawView is not defined by
  # NSQuickDrawView.h when compiling in 64-bit mode, and 64-bit mode is the
  # default when compiling on Snow Leopard.
  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL i386)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i386")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386")
  endif()

  # The Mac port simply uses too many deprecated things.
  if(COMPILER_GCC)
	set(WFLAGS "${WFLAGS} -Wno-deprecated-declarations")
  endif(COMPILER_GCC)
endif(APPLE)

if(UNIX AND NOT APPLE AND NOT BEOS)
  set(PLAT_UNIX 1)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(PLAT_LINUX 1)
endif()

if(PSP)
  set(PLAT_PSP 1)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -G0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -G0 -fno-exceptions -fno-rtti")
endif(PSP)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WFLAGS} ${WFLAGS_C_ONLY}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WFLAGS}")

if(NOT BUILD_SHARED_LIBS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_STATICLINK")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_STATICLINK")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUGMODE=1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUGMODE=1")

list(APPEND CMAKE_BUILD_TYPES Profile)
mark_as_advanced(
  CMAKE_C_FLAGS_PROFILE
  CMAKE_CXX_FLAGS_PROFILE
  CMAKE_EXE_LINKER_FLAGS_PROFILE
  )

if(COMPILER_GCC)
  set(CMAKE_C_FLAGS_PROFILE "-pg"
	CACHE STRING "profiling flags")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE}"
	CACHE STRING "profiling flags")
  set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg"
	CACHE STRING "profiling flags")
endif(COMPILER_GCC)
if(COMPILER_MSVC)
  set(CMAKE_C_FLAGS_PROFILE "-Gd -Ox"
	CACHE STRING "profiling flags")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE}"
	CACHE STRING "profiling flags")
  set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-profile"
	CACHE STRING "profiling flags")
endif(COMPILER_MSVC)

# message("C_FLAGS is ${CMAKE_C_FLAGS}")
# message("CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

#-----------------------------------------------------------------------------#
#
# Custom functions
#

# For OS X frameworks to work you must add headers to the target's sources.
function(add_our_library target)
  add_library(${target} ${ARGN})
  set_target_properties(${target}
	PROPERTIES
	DEBUG_POSTFIX -debug
	PROFILE_POSTFIX -profile
	)
endfunction(add_our_library)

function(set_our_framework_properties target nm)
  if(WANT_FRAMEWORKS)
	if(WANT_EMBED)
	  set(install_name_dir "@executable_path/../Frameworks")
	else()
	  set(install_name_dir "${FRAMEWORK_INSTALL_PREFIX}")
	endif(WANT_EMBED)
	set_target_properties(${target}
	  PROPERTIES
	  FRAMEWORK on
	  OUTPUT_NAME ${nm}
	  INSTALL_NAME_DIR "${install_name_dir}"
	  )
  endif(WANT_FRAMEWORKS)
endfunction(set_our_framework_properties)

function(install_our_library target)
  install(TARGETS ${target}
	PERMISSIONS WORLD_READ OWNER_WRITE GROUP_WRITE
	LIBRARY DESTINATION "lib${LIB_SUFFIX}"
	ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
	FRAMEWORK DESTINATION "${FRAMEWORK_INSTALL_PREFIX}"
	RUNTIME DESTINATION "bin"
	# Doesn't work, see below.
	# PUBLIC_HEADER DESTINATION "include"
	)
endfunction(install_our_library)

# Unfortunately, CMake's PUBLIC_HEADER support doesn't install into nested
# directories well, otherwise we could rely on install(TARGETS) to install
# header files associated with the target.  Instead we use the install(FILES)
# to install headers.  We reuse the MACOSX_PACKAGE_LOCATION property,
# substituting the "Headers" prefix with "include".
function(install_our_headers)
  foreach(hdr ${ARGN})
	get_source_file_property(LOC ${hdr} MACOSX_PACKAGE_LOCATION)
	string(REGEX REPLACE "^Headers" "include" LOC ${LOC})
	install(FILES ${hdr} DESTINATION ${LOC})
  endforeach()
endfunction(install_our_headers)

FUNCTION(RM_DUP_LIB LIBS_VAR LIB_DIRS_VAR)
  SET(_LIBS)
  SET(_LIB_DIRS)
  FOREACH(lib ${ARGN})
	# Watch out for -framework options (OS X)
	IF (NOT lib MATCHES "-framework.*|.*framework")
	  # Remove absolute path and add it to _LIB_DIRS.
	  IF(lib MATCHES "/.*")
		SET(abs_path)
		STRING(REGEX REPLACE "(/.*)/.*" "\\1" abs_path ${lib})
		LIST(APPEND _LIB_DIRS ${abs_path})
		STRING(REGEX REPLACE "/.*/(.*)" "\\1" lib ${lib})
	  ENDIF()

	  # Remove .a/.so/.dylib.
	  STRING(REGEX REPLACE "lib(.*)\\.(a|so|dylib)" "\\1" lib ${lib})

	  # Remove -l prefix if it's there already.
	  STRING(REGEX REPLACE "-l(.*)" "\\1" lib ${lib})

	  LIST(APPEND _LIBS "${lib}")
	ENDIF()
  ENDFOREACH()
  LIST(REVERSE _LIBS)
  LIST(REMOVE_DUPLICATES _LIBS)
  LIST(REVERSE _LIBS)
  SET(${LIBS_VAR} ${_LIBS} PARENT_SCOPE)
  LIST(REMOVE_DUPLICATES _LIB_DIRS)
  SET(${LIB_DIRS_VAR} ${_LIB_DIRS} PARENT_SCOPE)
ENDFUNCTION()

function(add_copy_file outputs from to)
  if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
	return()
  endif()

  if(NOT to)
	set(to ${from})
  endif()

  add_custom_command(
	OUTPUT  "${to}"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${from}"
	COMMAND "${CMAKE_COMMAND}" -E copy
	"${CMAKE_CURRENT_SOURCE_DIR}/${from}"
	"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${to}")
  list(APPEND ${outputs} ${to})
  set(${outputs} ${${outputs}} PARENT_SCOPE)
  # message("${outputs} ${${outputs}} ${from} ${to}")
endfunction()
