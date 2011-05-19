# Following macros are exported
#
# - merge_static_libs(target
#     [OUTPUT_NAME output_name]
#     LIBRARIES lib1 ... libN)
# This macro merges several static libraries into a single one.

if(NOT COMMAND merge_static_libs)
  include(CMakeParseArguments)

  # Merge static libraries into a big static lib. The resulting library
  # should not not have dependencies on other static libraries.
  function(merge_static_libs target)
	set(options)
	set(oneValueArgs OUTPUT_NAME)
	set(multiValueArgs LIBRARIES)
	cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	# To produce a library we need at least one source file.
	# It is created by ADD_CUSTOM_COMMAND below and will helps
	# also help to track dependencies.
	set(source_file ${CMAKE_CURRENT_BINARY_DIR}/${target}_depends.c)
	add_library(${target} STATIC ${source_file})

	if(arg_OUTPUT_NAME)
	  set_target_properties(${target} PROPERTIES OUTPUT_NAME ${arg_OUTPUT_NAME})
	endif()

	set(oslibs)
	set(static_libs)
	foreach(lib ${arg_LIBRARIES})
	  get_target_property(lib_location ${lib} LOCATION_${CMAKE_BUILD_TYPE})
	  get_target_property(lib_type ${lib} TYPE)
	  if(NOT lib_location)
		list(APPEND oslibs ${lib})
	  else()
		# This is a target in current project
		# (can be a static or shared lib)
		if(lib_type STREQUAL "STATIC_LIBRARY")
		  list(APPEND static_libs ${lib_location})
		  add_dependencies(${target} ${lib})
		endif()
	  endif()
	endforeach()
	if(OSLIBS)
	  list(REMOVE_DUPLICATES OSLIBS)
	  target_link_libraries(${TARGET} ${OSLIBS})
	endif()

	# Make the generated dummy source file depended on all static input
	# libs. If input lib changes,the source file is touched
	# which causes the desired effect (relink).
	add_custom_command(
	  OUTPUT  ${source_file}
	  COMMAND ${CMAKE_COMMAND}  -E touch ${source_file}
	  DEPENDS ${static_libs})

	if(MSVC)
	  # To merge libs, just pass them to lib.exe command line.
	  set(linker_extra_flags "")
	  foreach(lib ${static_libs})
		set(linker_extra_flags "${linker_extra_flags} ${lib}")
	  endforeach()
	  set_target_properties(${target} PROPERTIES STATIC_LIBRARY_FLAGS
		"${linker_extra_flags}")
	else()
	  get_target_property(target_location ${target} LOCATION_${CMAKE_BUILD_TYPE})
	  if(APPLE)
		# Use OSX's libtool to merge archives (ihandles universal
		# binaries properly)
		add_custom_command(TARGET ${target} POST_BUILD
		  COMMAND rm ${target_location}
		  COMMAND /usr/bin/libtool -static -o ${target_location}
		  ${static_libs}
		  )
	  else()
		# Generic Unix, Cygwin or MinGW. In post-build step, call
		# script, that extracts objects from archives with "ar x"
		# and repacks them with "ar r"
		configure_file(
		  ${CMAKE_TEMPLATE_PATH}/merge_archives_unix.cmake.in
		  ${CMAKE_CURRENT_BINARY_DIR}/merge_archives_${target}.cmake
		  @ONLY
		  )
		add_custom_command(TARGET ${target} POST_BUILD
		  COMMAND rm ${target_location}
		  COMMAND ${CMAKE_COMMAND} -P
		  ${CMAKE_CURRENT_BINARY_DIR}/merge_archives_${target}.cmake
		  )
	  endif()
	endif()
  endfunction()
endif()
