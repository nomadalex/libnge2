#######################################################################
# This file takes care of lib depends for target.
#######################################################################

if(NOT COMMAND RM_DUP_LIB)
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

  FUNCTION(PRINT_LIST var)
	SET(_STR)
	FOREACH(v ${${var}})
	  SET(_STR "${_STR} ${v}")
	ENDFOREACH()
	MESSAGE(${var} ": " ${_STR})
  ENDFUNCTION()
endif()
