#-----------------------------------------------------------------------------#
# Build options

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
	"Choose the type of build, options are:
	None Debug Release RelWithDebInfo MinSizeRel Profile."
	FORCE)
endif()

#-----------------------------------------------------------------------------#
# Common Compiler and platform setup

# Use relative paths
# This is mostly to reduce path size for command-line limits on windows
if(WIN32)
  # This seems to break Xcode projects so definitely don't enable on Apple builds
  set(CMAKE_USE_RELATIVE_PATHS true)
  set(CMAKE_SUPPRESS_REGENERATION true)
endif()

if (NOT APPLE)
  # Create debug libraries with _d postfix
  set(CMAKE_DEBUG_POSTFIX "_d")
endif ()

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

if(APPLE)
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

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(LINUX 1)
endif()

if(PSP)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -G0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -G0 -fno-exceptions -fno-rtti")
endif(PSP)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WFLAGS} ${WFLAGS_C_ONLY}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WFLAGS}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

#-----------------------------------------------------------------------------#
# Custom functions

macro(add_our_library target type)
  string(TOUPPER ${target} name)
  SET(${name}_TYPE ${type})
  add_library(${target} ${${name}_TYPE} ${ARGN})
  IF(${${name}_TYPE} STREQUAL STATIC)

	if(${CMAKE_CURRENT_LIST_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	  SET(${name}_LIBS ${${name}_LIBS})
	else()
	  SET(${name}_LIBS ${${name}_LIBS} PARENT_SCOPE)
	endif()

	foreach(lib ${${name}_LIBS})
	  if(TARGET ${lib})
		add_dependencies(${target} ${lib})
	  endif()
	endforeach()
  else()
	target_link_libraries(${target} ${${name}_LIBS})
  ENDIF()
  # message("name: " ${name} "\nLIBS: " ${${name}_LIBS})
endmacro()

function(install_our_target target)
  install(TARGETS ${target}
	PERMISSIONS WORLD_READ OWNER_WRITE GROUP_WRITE
	LIBRARY DESTINATION "lib${LIB_SUFFIX}"
	ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
	FRAMEWORK DESTINATION "${FRAMEWORK_INSTALL_PREFIX}"
	RUNTIME DESTINATION "bin"
	)
endfunction()
