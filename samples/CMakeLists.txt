include(RmDupLibs)
function(add_nge_executable nm)
  set(_ARGS)
  set(_LIBS)
  set(_LIBS_PRE)
  set(_IS_LIB)
  set(_IS_LIB_PRE)

  foreach(arg ${ARGN})
	if(${arg} STREQUAL "LIBRARIES")
	  set(_IS_LIB 1)
	  set(_IS_LIB_PRE)
	elseif(${arg} STREQUAL "LIBRARIES_PRE")
	  set(_IS_LIB)
	  set(_IS_LIB_PRE 1)
	else()
	  if(_IS_LIB)
		list(APPEND _LIBS ${arg})
	  elseif(_IS_LIB_PRE)
		list(APPEND _LIBS_PRE ${arg})
	  else()
		list(APPEND _ARGS ${arg})
	  endif()
	endif()
  endforeach()

  if(ANDROID)
	add_library(${nm} SHARED ${NGE_ANDROID_JNI_FILE} ${_ARGS})
	set(_LIBS ${_LIBS_PRE} nge2 ${_LIBS} ${NGE2_LIBS})
  else()
	add_executable(${nm} WIN32 ${_ARGS})
	set(_LIBS ${_LIBS_PRE} nge2_main nge2 ${_LIBS} ${NGE2_LIBS})
  endif()

  rm_dup_lib(_LIBS)
  target_link_libraries(${nm} ${_LIBS})
  if(PSP)
	pack_psp_eboot(${nm})
  elseif(ANDROID)
	if(${NGE2_BUILD_TYPE} STREQUAL Debug)
	  set(LIB_OUTPUT_NAME lib${nm}${CMAKE_DEBUG_POSTFIX})
	elseif(${NGE2_BUILD_TYPE} STREQUAL RelWithDebInfo)
	  set(LIB_OUTPUT_NAME lib${nm}${CMAKE_RELWITHDEBINFO_POSTFIX})
	elseif(${NGE2_BUILD_TYPE} STREQUAL MinSizeRel)
	  set(LIB_OUTPUT_NAME lib${nm}${CMAKE_MINSIZEREL_POSTFIX})
	else()
	  set(LIB_OUTPUT_NAME lib${nm}${CMAKE_RELEASE_POSTFIX})
	endif()

	execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory
	  "${CMAKE_SOURCE_DIR}/Android/assets"
	  "${nm}/assets"
	  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

	execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory
	  "${CMAKE_SOURCE_DIR}/Android/res"
	  "${nm}/res"
	  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	configure_file("${CMAKE_SOURCE_DIR}/Android/strings.xml.in"
	  "${nm}/res/values/strings.xml" @ONLY)
	set(project_name "${nm}")

	configure_file("${CMAKE_SOURCE_DIR}/Android/default.properties"
	  "${nm}/default.properties" COPYONLY)
	set(package_name "org.libnge.nge2.${nm}")
	configure_file("${CMAKE_SOURCE_DIR}/Android/AndroidManifest.xml.in"
	  "${nm}/AndroidManifest.xml" @ONLY)
	set(app_name "${nm}")
	configure_file("${CMAKE_SOURCE_DIR}/Android/project_create.sh.in"
	  "${nm}/project_create.sh" @ONLY)

	copy_coolaudio_android_files(${nm} ${CMAKE_CURRENT_BINARY_DIR}/${nm})
	copy_nge_android_java_files(${nm} ${CMAKE_CURRENT_BINARY_DIR}/${nm})

	add_custom_command(
	  TARGET ${nm}
	  POST_BUILD
	  COMMAND "${CMAKE_COMMAND}" -E copy
	  "${LIBRARY_OUTPUT_PATH}/${LIB_OUTPUT_NAME}.so"
	  "${CMAKE_CURRENT_BINARY_DIR}/${nm}/libs/${ARMEABI_NDK_NAME}/libnge2app-jni.so"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${nm}"
	  )
  endif()
endfunction()

function(add_resource all_res dir)
  set(res)
  foreach(arg ${ARGN})
	list(APPEND res ${dir}/${arg})
  endforeach()
  list(APPEND ${all_res} ${res})
  set(${all_res} ${${all_res}} PARENT_SCOPE)
endfunction()

include(CopyFile)
function(copy_res_files target)
  set(files)

  if(ANDROID)
	set(prefix Resources/)
  else()
	set(prefix "")
  endif()

  foreach(arg ${ARGN})
	add_copy_file(files ../res/${arg} ${prefix}${arg})
  endforeach()
  # message("files : ${files}")
  add_custom_target(${target}_res DEPENDS ${files})
  add_dependencies(${target} ${target}_res)
endfunction()

if(NOT ANDROID)
  #demo 3 need pmp mod library.
  foreach(i 0 1 2 4 6 7 8)
	add_subdirectory(demo${i})
  endforeach()
endif()

foreach(i 10 11 12 13 14)
 add_subdirectory(demo${i})
endforeach()

if(NOT ANDROID)
  if(UNZIP_FOUND)
	add_subdirectory(demo9)
  endif()
endif()
