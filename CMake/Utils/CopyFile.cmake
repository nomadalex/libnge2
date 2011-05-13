################################################################################
# this file provide function which can copy file on build time.
################################################################################

if(NOT COMMAND add_copy_file)
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
  endfunction()
endif()
