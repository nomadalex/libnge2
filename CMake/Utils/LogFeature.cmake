################################################################################
# This file defines the Feature Logging Utils.
#
# LOG_FEATURE(VAR FEATURE DESCRIPTION URL [REQUIRED [MIN_VERSION [COMMENTS]]])
#   Logs the information so that it can be displayed at the end
#   of the configure run
#   VAR : TRUE or FALSE, indicating whether the feature is supported
#   FEATURE: name of the feature, e.g. "libjpeg"
#   DESCRIPTION: description what this feature provides
#   URL: home page
#   REQUIRED: TRUE or FALSE, indicating whether the featue is required
#   MIN_VERSION: minimum version number. empty string if unneeded
#   COMMENTS: More info you may want to provide.  empty string if unnecessary
#
# DISPLAY_FEATURE_LOG()
#   Call this to display the collected results.
#   Exits CMake with a FATAL error message if a required feature is missing
#
# Example:
#
# INCLUDE(MacroLogFeature)
#
# FIND_PACKAGE(JPEG)
# LOG_FEATURE(JPEG_FOUND "libjpeg" "Support JPEG images" "http://www.ijg.org" TRUE "3.2a" "")
# ...
# DISPLAY_FEATURE_LOG()

IF (NOT _logFeatureAlreadyIncluded)
   SET(_file ${CMAKE_BINARY_DIR}/MissingRequirements.txt)
   IF (EXISTS ${_file})
      FILE(REMOVE ${_file})
   ENDIF ()

   SET(_file ${CMAKE_BINARY_DIR}/EnabledFeatures.txt)
   IF (EXISTS ${_file})
      FILE(REMOVE ${_file})
   ENDIF ()

   SET(_file ${CMAKE_BINARY_DIR}/DisabledFeatures.txt)
   IF (EXISTS ${_file})
      FILE(REMOVE ${_file})
  ENDIF ()

  SET(_logFeatureAlreadyIncluded TRUE)
ENDIF ()

function(LOG_FEATURE _var _package _description _url)
  if("${ARGV4}" STREQUAL "REQUIRED")
	set(_required 1)
	SET(_minvers "${ARGV5}")
	SET(_comments "${ARGV6}")
  else()
	set(_required)
	SET(_minvers "${ARGV4}")
	SET(_comments "${ARGV5}")
  endif()

   IF (${_var})
     SET(_LOGFILENAME ${CMAKE_BINARY_DIR}/EnabledFeatures.txt)
   ELSE (${_var})
     IF (_required)
       SET(_LOGFILENAME ${CMAKE_BINARY_DIR}/MissingRequirements.txt)
     ELSE ()
       SET(_LOGFILENAME ${CMAKE_BINARY_DIR}/DisabledFeatures.txt)
     ENDIF ()
   ENDIF ()

   SET(_logtext "+ ${_package}")

   IF (NOT ${_var})
      IF (${_minvers} MATCHES ".*")
        SET(_logtext "${_logtext}, ${_minvers}")
      ENDIF (${_minvers} MATCHES ".*")
      SET(_logtext "${_logtext}: ${_description} <${_url}>")
      IF (${_comments} MATCHES ".*")
        SET(_logtext "${_logtext}\n${_comments}")
      ENDIF (${_comments} MATCHES ".*")
#      SET(_logtext "${_logtext}\n") #double-space missing features?
   ENDIF (NOT ${_var})
   FILE(APPEND "${_LOGFILENAME}" "${_logtext}\n")
ENDFUNCTION()

function(DISPLAY_FEATURE_LOG)
   SET(_file ${CMAKE_BINARY_DIR}/MissingRequirements.txt)
   IF (EXISTS ${_file})
      FILE(READ ${_file} _requirements)
      MESSAGE(FATAL_ERROR "\n-----------------------------------------------------------------------------\n-- The following REQUIRED packages could NOT be located on your system.\n-- Please install them before continuing this software installation.\n${MISS_FEATURE_CUSTOM_MSG}\n-----------------------------------------------------------------------------\n${_requirements}-----------------------------------------------------------------------------")
      FILE(REMOVE ${_file})
      MESSAGE(FATAL_ERROR "Exiting: Missing Requirements")
   ENDIF ()

   SET(_summary "\n")

   SET(_elist 0)
   SET(_file ${CMAKE_BINARY_DIR}/EnabledFeatures.txt)
   IF (EXISTS ${_file})
      SET(_elist 1)
      FILE(READ ${_file} _enabled)
      FILE(REMOVE ${_file})
      SET(_summary "${_summary}-----------------------------------------------------------------------------\n-- The following external packages were located on your system.\n-- This installation will have the extra features provided by these packages.\n-----------------------------------------------------------------------------\n${_enabled}")
   ENDIF ()

   SET(_dlist 0)
   SET(_file ${CMAKE_BINARY_DIR}/DisabledFeatures.txt)
   IF (EXISTS ${_file})
      SET(_dlist 1)
      FILE(READ ${_file} _disabled)
      FILE(REMOVE ${_file})
      SET(_summary "${_summary}-----------------------------------------------------------------------------\n-- The following OPTIONAL packages could NOT be located on your system.\n-- Consider installing them to enable more features from this software.\n${MISS_FEATURE_CUSTOM_MSG}\n-----------------------------------------------------------------------------\n${_disabled}")
   ELSE ()
      IF (${_elist})
        SET(_summary "${_summary}Congratulations! All external packages have been found.\n")
      ENDIF ()
   ENDIF ()

   IF (${_elist} OR ${_dlist})
      SET(_summary "${_summary}-----------------------------------------------------------------------------\n")
   ENDIF ()
   MESSAGE(STATUS "${_summary}")
endfunction()
