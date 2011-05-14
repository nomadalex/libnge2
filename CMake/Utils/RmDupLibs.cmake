#######################################################################
# This file takes care of lib depends for target.
#######################################################################

if(NOT COMMAND RM_DUP_LIB)
  macro(check_specifier var prefix)
	set(${prefix})
	foreach(_spec "optimized" "debug")
	  set(${prefix}_${_spec})
	  if(${${var}} STREQUAL ${_spec})
		set(${prefix} 1)
		set(${prefix}_${_spec} 1)
		break()
	  endif()
	endforeach()
  endmacro()

  function(pick_out_lib lib return)
	# Watch out for -framework options (OS X)
	IF (NOT lib MATCHES "-framework.*|.*framework")
	  # Remove absolute path and add it to _LIB_DIRS.
	  IF(lib MATCHES "/.*")
		STRING(REGEX REPLACE "/.*/(.*)" "\\1" lib ${lib})
	  ENDIF()

	  # Remove .a/.so/.dylib.
	  STRING(REGEX REPLACE "lib(.*)\\.(a|so|dylib)" "\\1" lib ${lib})

	  # Remove -l prefix if it's there already.
	  STRING(REGEX REPLACE "-l(.*)" "\\1" lib ${lib})
	ENDIF()
	set(${return} ${lib} PARENT_SCOPE)
  endfunction()

  macro(check_lib)
	set(_llib)
	check_specifier(_lib _skip)
	if(_skip) # spec lib
	  set(_spec ${_lib})

	  math(EXPR _idx "${_idx} + 1")
	  list(GET _libs ${_idx} _lib)

	  pick_out_lib(${_lib} _llib)
	  set("_lib_var_${_llib}_${_spec}" ${_spec} ${_lib})
	  list(APPEND _lib_vars "_lib_var_${_llib}_${_spec}")
	else()  # normal lib
	  pick_out_lib(${_lib} _llib)
	  set("_lib_var_${_llib}" ${_lib})
	  list(APPEND _lib_vars "_lib_var_${_llib}")
	endif()
  endmacro()

  function(rm_dup_lib libs_var)
	set(_libs ${${libs_var}})
	set(_lib_vars)
	set(_idx 0)
	list(LENGTH _libs _max_idx)
	while(_idx LESS _max_idx)
	  list(GET _libs ${_idx} _lib)
	  if(_lib)
		check_lib()
	  else()
		break()
	  endif()
	  math(EXPR _idx "${_idx} + 1")
	endwhile()

	list(REVERSE _lib_vars)
	list(REMOVE_DUPLICATES _lib_vars)
	list(REVERSE _lib_vars)

	set(_libs)
	foreach(_var ${_lib_vars})
	  list(APPEND _libs ${${_var}})
	endforeach()
	set(${libs_var} ${_libs} PARENT_SCOPE)
  endfunction()
endif()
