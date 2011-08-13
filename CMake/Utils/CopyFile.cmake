################################################################################
# this file provide function which can copy file on build time.
################################################################################

if(NOT COMMAND add_copy_file)
  function(copy_file output fromfile tofile)
	add_custom_command(
	  OUTPUT  "${output}"
	  DEPENDS "${fromfile}"
	  COMMAND "${CMAKE_COMMAND}" -E copy
	  "${fromfile}"
	  "${tofile}"
	  )
  endfunction()

  function(add_copy_file outputs from to)
	if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
	  return()
	endif()

	if(NOT to)
	  set(to ${from})
	endif()

	copy_file(${to} "${CMAKE_CURRENT_SOURCE_DIR}/${from}" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${to}")
	list(APPEND ${outputs} ${to})
	set(${outputs} ${${outputs}} PARENT_SCOPE)
  endfunction()
endif()
