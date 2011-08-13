include(RmDupLibs)
function(add_nge_executable nm)
  set(_ARGS)
  set(_LIBS)
  set(_IS_LIB)

  foreach(arg ${ARGN})
	if(_IS_LIB)
	  list(APPEND _LIBS ${arg})
	else()
	  if(${arg} STREQUAL "LIBRARIES")
		set(_IS_LIB 1)
	  else()
		list(APPEND _ARGS ${arg})
	  endif()
	endif()
  endforeach()

  if(ANDROID)
	add_library(${nm} SHARED ${NGE_ANDROID_JNI_FILE} ${_ARGS})
  else()
	add_executable(${nm} WIN32 ${_ARGS})
  endif()

  set(_LIBS nge2 ${_LIBS} ${NGE2_LIBS})

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

	copy_coolaudio_android_java_files(${nm} ${CMAKE_CURRENT_BINARY_DIR}/android)
	copy_nge_android_java_files(${nm} ${CMAKE_CURRENT_BINARY_DIR}/android)

	configure_file("${CMAKE_SOURCE_DIR}/Android/default.properties"
	  "android/default.properties" COPYONLY)
	set(package_name "org.libnge.nge2.${nm}")
	configure_file("${CMAKE_SOURCE_DIR}/Android/AndroidManifest.xml.in"
	  "android/AndroidManifest.xml" @ONLY)
	set(app_name "${nm}")
	configure_file("${CMAKE_SOURCE_DIR}/Android/strings.xml.in"
	  "android/res/values/strings.xml" @ONLY)
	set(project_name "${nm}")
	configure_file("${CMAKE_SOURCE_DIR}/Android/project_create.sh.in"
	  "android/project_create.sh" @ONLY)

	add_custom_command(
	  TARGET ${nm}
	  POST_BUILD
	  COMMAND "${CMAKE_COMMAND}" -E copy
	  "${LIBRARY_OUTPUT_PATH}/${LIB_OUTPUT_NAME}.so"
	  "${CMAKE_CURRENT_BINARY_DIR}/android/libs/${ARMEABI_NDK_NAME}/libnge2app-jni.so"
	  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/android/project_create.sh"
	  COMMAND "ant" debug
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/android"
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
	set(prefix res/)
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

foreach(i 10 11)
  add_subdirectory(demo${i})
endforeach()

if(NOT ANDROID)
  if(SQLITE3_FOUND)
	add_subdirectory(demo5)
  endif()

  if(UNZIP_FOUND)
	add_subdirectory(demo9)
  endif()
endif()
