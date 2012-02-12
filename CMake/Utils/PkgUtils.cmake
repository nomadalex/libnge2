##################################################################
# Provides some common functionality for the FindPackage modules
##################################################################

# Get environment variable, define it as ENV_$var and make sure backslashes are converted to forward slashes
macro(getenv_path VAR)
   set(ENV_${VAR} $ENV{${VAR}})
   # replace won't work if var is blank
   if (ENV_${VAR})
     string( REGEX REPLACE "\\\\" "/" ENV_${VAR} ${ENV_${VAR}} )
   endif ()
endmacro(getenv_path)

# Display a status message unless FIND_QUIETLY is set
function(pkg_message PREFIX)
  if (NOT ${PREFIX}_FIND_QUIETLY)
    message(STATUS ${ARGN})
  endif ()
endfunction(pkg_message)

# Construct search paths for includes and libraries from a PREFIX_PATH
macro(create_search_paths PREFIX)
  foreach(dir ${${PREFIX}_PREFIX_PATH})
    list(APPEND ${PREFIX}_INC_SEARCH_PATH
      ${dir}/include ${dir}/Include ${dir}/include/${PREFIX} ${dir}/Headers)
    list(APPEND ${PREFIX}_LIB_SEARCH_PATH
      ${dir}/lib ${dir}/Lib ${dir}/lib/${PREFIX} ${dir}/Libs)
    list(APPEND ${PREFIX}_BIN_SEARCH_PATH
      ${dir}/bin)
  endforeach(dir)
  set(${PREFIX}_FRAMEWORK_SEARCH_PATH ${${PREFIX}_PREFIX_PATH})
endmacro(create_search_paths)

# clear cache variables if a certain variable changed
macro(clear_if_changed TESTVAR)
  # test against internal check variable
  # HACK: Apparently, adding a variable to the cache cleans up the list
  # a bit. We need to also remove any empty strings from the list, but
  # at the same time ensure that we are actually dealing with a list.
  list(APPEND ${TESTVAR} "")
  list(REMOVE_ITEM ${TESTVAR} "")
  if (NOT "${${TESTVAR}}" STREQUAL "${${TESTVAR}_INT_CHECK}")
    message(STATUS "${TESTVAR} changed.")
    foreach(var ${ARGN})
	  get_property(_docstring CACHE ${var} PROPERTY HELPSTRING)
      set(${var} "NOTFOUND" CACHE STRING "${_docstring}" FORCE)
    endforeach(var)
  endif ()
  set(${TESTVAR}_INT_CHECK ${${TESTVAR}} CACHE INTERNAL "x" FORCE)
endmacro(clear_if_changed)

# Try to get some hints from pkg-config, if available
macro(use_pkgconfig PREFIX PKGNAME)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(${PREFIX} ${PKGNAME})
  endif ()
endmacro (use_pkgconfig)

# Couple a set of release AND debug libraries (or frameworks)
macro(make_library_set PREFIX)
  if (${PREFIX}_FWK)
    set(${PREFIX} ${${PREFIX}_FWK})
  elseif (${PREFIX}_REL AND ${PREFIX}_DBG)
    set(${PREFIX} optimized ${${PREFIX}_REL} debug ${${PREFIX}_DBG})
  elseif (${PREFIX}_REL)
    set(${PREFIX} ${${PREFIX}_REL})
  elseif (${PREFIX}_DBG)
    set(${PREFIX} ${${PREFIX}_DBG})
  endif ()
endmacro(make_library_set)

# Generate debug names from given release names
macro(make_debug_names PREFIX)
  if(NOT DEBUG_POSTFIXS)
	set(DEBUG_POSTFIXS "d" "D" "_d" "_D" "_debug")
  endif()

  foreach(i ${${PREFIX}})
	foreach(_fix ${DEBUG_POSTFIXS})
	  list(APPEND ${PREFIX}_DBG "${i}${_fix}")
	endforeach()
	list(APPEND ${PREFIX}_DBG "${i}")
  endforeach(i)
endmacro()

# Add the parent dir from DIR to VAR
macro(add_parent_dir VAR DIR)
  get_filename_component(${DIR}_TEMP "${${DIR}}/.." ABSOLUTE)
  set(${VAR} ${${VAR}} ${${DIR}_TEMP})
endmacro(add_parent_dir)

# Slightly customised framework finder
MACRO(findpkg_framework fwk)
  IF(APPLE)
    SET(${fwk}_FRAMEWORK_PATH
      ${${fwk}_FRAMEWORK_SEARCH_PATH}
      ${CMAKE_FRAMEWORK_PATH}
      ~/Library/Frameworks
      /Library/Frameworks
      /System/Library/Frameworks
      /Network/Library/Frameworks
      /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk/System/Library/Frameworks/
      ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release
      ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug
    )
    # These could be arrays of paths, add each individually to the search paths
    foreach(i ${FRAMEWORK_PREFIX_PATH})
      set(${fwk}_FRAMEWORK_PATH ${${fwk}_FRAMEWORK_PATH} ${i}/lib/Release ${i}/lib/Debug)
    endforeach(i)

    foreach(i ${FRAMEWORK_PREFIX_BUILD})
      set(${fwk}_FRAMEWORK_PATH ${${fwk}_FRAMEWORK_PATH} ${i}/lib/Release ${i}/lib/Debug)
    endforeach(i)

    FOREACH(dir ${${fwk}_FRAMEWORK_PATH})
      SET(fwkpath ${dir}/${fwk}.framework)
      IF(EXISTS ${fwkpath})
        SET(${fwk}_FRAMEWORK_INCLUDES ${${fwk}_FRAMEWORK_INCLUDES}
          ${fwkpath}/Headers ${fwkpath}/PrivateHeaders)
        SET(${fwk}_FRAMEWORK_PATH ${dir})
        if (NOT ${fwk}_LIBRARY_FWK)
          SET(${fwk}_LIBRARY_FWK "-framework ${fwk}")
        endif ()
      ENDIF(EXISTS ${fwkpath})
    ENDFOREACH(dir)
  ENDIF(APPLE)
ENDMACRO(findpkg_framework)

macro(prepare_dep_search PREFIX)
  set(${PREFIX}_DEPENDENCIES_DIR "" CACHE PATH "Path to prebuilt dependencies")
  getenv_path(${PREFIX}_DEPENDENCIES_DIR)
  set(${PREFIX}_DEP_SEARCH_PATH
	${${PREFIX}_DEPENDENCIES_DIR}
	${ENV_${PREFIX}_DEPENDENCIES_DIR}
	)

  # Set hardcoded path guesses for various platforms
  if(WIN32)
	list(APPEND ${PREFIX}_DEP_SEARCH_PATH "${CMAKE_SOURCE_DIR}/Win32Depends")
  elseif(ANDROID)
	list(APPEND ${PREFIX}_DEP_SEARCH_PATH "${CMAKE_SOURCE_DIR}/AndroidDepends")
  elseif(UNIX)
	list(APPEND ${PREFIX}_DEP_SEARCH_PATH "/usr/local")
  endif()
  message(STATUS "Search path: ${${PREFIX}_DEP_SEARCH_PATH}")

  # give guesses as hints to the find_package calls
  set(CMAKE_PREFIX_PATH ${${PREFIX}_DEP_SEARCH_PATH} ${CMAKE_PREFIX_PATH})
  set(CMAKE_FRAMEWORK_PATH ${${PREFIX}_DEP_SEARCH_PATH} ${CMAKE_FRAMEWORK_PATH})
endmacro()

macro(add_package PKG_FIX LIBS_VAR)
  if(${PKG_FIX}_INCLUDE_DIRS)
	include_directories(${${PKG_FIX}_INCLUDE_DIRS})
  elseif(${PKG_FIX}_INCLUDE_DIR)
	include_directories(${${PKG_FIX}_INCLUDE_DIR})
  endif()
  if(${PKG_FIX}_LIBRARY_DIRS)
	link_directories(${${PKG_FIX}_LIBRARY_DIRS})
  elseif(${PKG_FIX}_LIBRARY_DIR)
	link_directories(${${PKG_FIX}_LIBRARY_DIR})
  endif()
  if(${PKG_FIX}_LIBRARIES)
	list(APPEND ${LIBS_VAR} ${${PKG_FIX}_LIBRARIES})
  elseif(${PKG_FIX}_LIBRARY)
	list(APPEND ${LIBS_VAR} ${${PKG_FIX}_LIBRARY})
  endif()
endmacro()

# Begin processing of package
function(findpkg_begin PREFIX)
  if (NOT ${PREFIX}_FIND_QUIETLY)
    message(STATUS "Looking for ${PREFIX}...")
  endif ()
endfunction(findpkg_begin)

# Do the final processing for the package find.
macro(findpkg_finish PREFIX)
  # skip if already processed during this run
  if (NOT ${PREFIX}_FOUND)
    if (${PREFIX}_INCLUDE_DIR AND ${PREFIX}_LIBRARY)
      set(${PREFIX}_FOUND TRUE)
      set(${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIR})
      set(${PREFIX}_LIBRARIES ${${PREFIX}_LIBRARY})
      if (NOT ${PREFIX}_FIND_QUIETLY)
        message(STATUS "Found ${PREFIX}: ${${PREFIX}_LIBRARIES}")
      endif ()
    else ()
      if (NOT ${PREFIX}_FIND_QUIETLY)
        message(STATUS "Could not locate ${PREFIX}")
      endif ()
      if (${PREFIX}_FIND_REQUIRED)
        message(FATAL_ERROR "Required library ${PREFIX} not found! Install the library (including dev packages) and try again. If the library is already installed, set the missing variables manually in cmake.")
      endif ()
    endif ()

    mark_as_advanced(${PREFIX}_INCLUDE_DIR ${PREFIX}_LIBRARY ${PREFIX}_LIBRARY_REL ${PREFIX}_LIBRARY_DBG ${PREFIX}_LIBRARY_FWK)
  endif ()
endmacro(findpkg_finish)

macro(prepare_find_pkg_vars PKG)
  # Get path, convert backslashes as ${ENV_${var}}
  getenv_path(${PKG}_HOME)

  # construct search paths
  set(${PKG}_PREFIX_PATH ${${PKG}_HOME} ${ENV_${PKG}_HOME})
  create_search_paths(${PKG})
  # redo search if prefix path changed
  clear_if_changed(${PKG}_PREFIX_PATH
	${PKG}_LIBRARY_FWK
	${PKG}_LIBRARY_REL
	${PKG}_LIBRARY_DBG
	${PKG}_INCLUDE_DIR
	)
endmacro()

include(CMakeParseArguments)
macro(define_find_pkg PKG)
  set(oneValueArgs PKG_CONFIG_NAME)
  set(multiValueArgs LIBRARY_HINTS HEADER_HINTS HEADER_SUFFIXES LIBRARY_SUFFIXES)
  cmake_parse_arguments(${PKG}_ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  prepare_find_pkg_vars(${PKG})
  set(${PKG}_LIBRARY_NAMES ${${PKG}_ARG_LIBRARY_HINTS})
  make_debug_names(${PKG}_LIBRARY_NAMES)

  if(${PKG}_ARG_PKG_CONFIG_NAME)
	use_pkgconfig(${PKG}_PKGC ${${PKG}_ARG_PKG_CONFIG_NAME})
  endif()
  findpkg_framework(${PKG} NGE2)

  if(${PKG}_ARG_HEADER_SUFFIXES)
	set(_suffixes ${${PKG}_ARG_HEADER_SUFFIXES})
  else()
	set(_suffixes)
  endif()
  find_path(${PKG}_INCLUDE_DIR NAMES ${${PKG}_ARG_HEADER_HINTS} HINTS ${${PKG}_INC_SEARCH_PATH} ${${PKG}_PKGC_INCLUDE_DIRS} PATH_SUFFIXES ${_suffixes} "")

  if(${PKG}_ARG_LIBRARY_SUFFIXES)
	set(_suffixes ${${PKG}_ARG_LIBRARY_SUFFIXES})
  else()
	set(_suffixes)
  endif()
  find_library(${PKG}_LIBRARY_REL NAMES ${${PKG}_LIBRARY_NAMES} HINTS ${${PKG}_LIB_SEARCH_PATH} ${${PKG}_PKGC_LIBRARY_DIRS} PATH_SUFFIXES ${_suffixes} "")
  find_library(${PKG}_LIBRARY_DBG NAMES ${${PKG}_LIBRARY_NAMES_DBG} HINTS ${${PKG}_LIB_SEARCH_PATH} ${${PKG}_PKGC_LIBRARY_DIRS} PATH_SUFFIXES ${_suffixes} "")
  make_library_set(${PKG}_LIBRARY)
endmacro()
