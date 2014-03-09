#-----------------------------------------------------------------------------#
# Build options

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
	"Choose the type of build, options are:
	None Debug Release RelWithDebInfo MinSizeRel Profile."
	FORCE)
endif()

# Set postfix
set(CMAKE_DEBUG_POSTFIX "_d" CACHE STRING "postfix for debug version")
set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "postfix for release version")
set(CMAKE_RELWITHDEBINFO_POSTFIX "_rd" CACHE STRING "postfix for release with debug info version")
set(CMAKE_MINSIZEREL_POSTFIX "_s" CACHE STRING "postfix for minsize release version")

mark_as_advanced(CMAKE_DEBUG_POSTFIX CMAKE_RELEASE_POSTFIX CMAKE_RELWITHDEBINFO_POSTFIX
  CMAKE_MINSIZEREL_POSTFIX)

set(INSTALL_PREFIX "")

option(STRICT_WARN "Halt at warnings" off)

#-----------------------------------------------------------------------------#
# Common Compiler and platform setup

# Use relative paths
# This is mostly to reduce path size for command-line limits on windows
if(WIN32)
  set(CMAKE_USE_RELATIVE_PATHS true)
  set(CMAKE_SUPPRESS_REGENERATION true)
endif()

if(PSP)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -G0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -G0 -fno-exceptions -fno-rtti")
elseif(WIN32)
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
  elseif(MSVC)
	set(COMPILER_MSVC 1)
	set(WFLAGS "/W3 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
	
	if(${_MACHINE_ARCH_FLAG} MATCHES X86)
	  SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
	  SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
	  SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
	endif()
	option(WIN32_USE_MP "Set to ON to build with the /MP option (Visual Studio 2005 and above)." OFF)
	mark_as_advanced(WIN32_USE_MP)
	if(WIN32_USE_MP)
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	endif()
  endif()
elseif(UNIX)
  if(CMAKE_SYSTEM_NAME STREQUAL Linux)
	set(LINUX 1)
  endif()
endif()

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WFLAGS} ${WFLAGS_C_ONLY}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WFLAGS}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

#-----------------------------------------------------------------------------#
# Custom functions

function(msvc_force_link_runtime target type)
  if(${type} STREQUAL "MT")
	set_target_properties(${target} PROPERTIES LINK_FLAGS "/nodefaultlib:libc /nodefaultlib:msvcrt /nodefaultlib:libcd /nodefaultlib:msvcrtd /nodefaultlib:libcmtd")
  elseif(${type} STREQUAL "MD")
	set_target_properties(${target} PROPERTIES LINK_FLAGS "/nodefaultlib:libc /nodefaultlib:libcmt /nodefaultlib:libcd /nodefaultlib:libcmtd /nodefaultlib:msvcrtd")
  elseif(${type} STREQUAL "MTd")
	set_target_properties(${target} PROPERTIES LINK_FLAGS "/nodefaultlib:libc /nodefaultlib:msvcrt /nodefaultlib:libcd /nodefaultlib:msvcrtd /nodefaultlib:libcmt")
  elseif(${type} STREQUAL "MDd")
	set_target_properties(${target} PROPERTIES LINK_FLAGS "/nodefaultlib:libc /nodefaultlib:libcmt /nodefaultlib:libcd /nodefaultlib:libcmtd /nodefaultlib:msvcrt")
  endif()
endfunction()

macro(add_our_library target type)
  string(TOUPPER ${target} name)
  SET(${name}_TYPE ${type})
  add_library(${target} ${${name}_TYPE} ${ARGN})
  IF(${${name}_TYPE} STREQUAL STATIC)
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
