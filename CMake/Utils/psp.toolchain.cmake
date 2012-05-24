# You will need at least CMake 2.6.0.
#
# Adjust the following paths to suit your environment.
#
# This file was based on http://www.cmake.org/Wiki/CmakeMingw

# The name of the target operating system.
set(CMAKE_SYSTEM_NAME Generic)

# Location of target environment.
find_program(psp-config_SCRIPT psp-config)
if (psp-config_SCRIPT)
  execute_process(COMMAND ${psp-config_SCRIPT}
	ARGS --psp-prefix
	OUTPUT_VARIABLE PSP_PREFIX
	OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${psp-config_SCRIPT}
	ARGS --pspsdk-path
	OUTPUT_VARIABLE PSPSDK_PATH
	OUTPUT_STRIP_TRAILING_WHITESPACE)
else (psp-config_SCRIPT)
  message(FATAL_ERROR "psp-config was not found.\nInstall the PSPDEV toolchain or update the $PATH environment variable.")
endif (psp-config_SCRIPT)

get_filename_component(PSP_BIN_PATH ${psp-config_SCRIPT} PATH CACHE)

# Which compilers to use for C and C++.
set(CMAKE_C_COMPILER psp-gcc)
set(CMAKE_CXX_COMPILER psp-g++)

# Needed to pass the compiler tests.
set(LINK_DIRECTORIES ${PSPSDK_PATH}/lib)
set(LINK_LIBRARIES -lc -lpspuser -lpspkernel -lc)

# Adjust the default behaviour of the FINDj_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment.
set(CMAKE_FIND_ROOT_PATH ${PSP_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

if(NOT PSP)
  set(PSP 1)
  include_directories(SYSTEM ${PSPSDK_PATH}/include)

  set(CMAKE_INSTALL_PREFIX ${PSPSDK_PATH} CACHE STRING
	"The prefix path for install.")
  message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

  set(SHARED FALSE CACHE BOOL
	"Build shared library." FORCE)

  # option :
  # KERNEL use kernel libs.
  # KERNEL_LIBC use kernel's libc // default libc is newlib's libc.
  # PSPSDK_LIBC use pspsdk's libc
  # USER use user libs.
  function(ADD_PSPSDK_LIBS lib_var)
	set(LIBC c)
	foreach(option ${ARGN})
	  if(option STREQUAL "KERNEL")
		set(PSP_KERNEL 1)
	  elseif(option STREQUAL "USER")
		set(PSP_USER 1)
	  elseif(option STREQUAL "KERNEL_LIBC")
		set(LIBC ${PSPSDK_PATH}/include/libc)
	  elseif(option STREQUAL "PSPSDK_LIBC")
		set(LIBC psplibc ${PSPSDK_PATH}/include/libc)
	  endif()
	endforeach()

	set(PSPSDK_LIBS ${${lib_var}} pspdebug)
	if(PSP_KERNEL)
	  list(APPEND PSPSDK_LIBS pspdisplay_driver pspctrl_driver pspsdk ${LIBC} pspkernel)
	else()
	  list(APPEND PSPSDK_LIBS pspdisplay pspge pspctrl pspsdk ${LIBC} pspnet pspnet_inet pspnet_apctl pspnet_resolver psputility pspuser)
	  if(NOT PSP_USER)
		list(APPEND PSPSDK_LIBS pspkernel)
	  endif()
	endif()
	set(${lib_var} ${PSPSDK_LIBS} PARENT_SCOPE)
  endfunction()

  # After building the ELF binary build the PSP executable.
  # FW_VERSION "Your psp fireware version."
  # SFO "Define the sfo for EBOOT.PBP."
  # ICON "Define the icon for EBOOT.PBP."
  # ICON1 "Define the icon1 for EBOOT.PBP."
  # UNKPNG "Define the unk png for EBOOT.PBP."
  # PIC1 "Define the pic1 for EBOOT.PBP."
  # SND0 "Define the snd0 for EBOOT.PBP."
  # PSAR "Define the psar for EBOOT.PBP."
  function(pack_psp_eboot nm)
	include(CMakeParseArguments)
	set(options LARGE_MEMORY)
	set(oneValueArgs FW_VERSION SFO ICON ICON1 UNKPNG PIC1 SND0 PSAR)
	set(multiValueArgs)
	cmake_parse_arguments(PSP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if(NOT PSP_FW_VERSION)
	  set(PSP_FW_VERSION 371)
	endif()
	add_definitions(-D_PSP_FW_VERSION=${PSP_FW_VERSION})

	if(PSP_LARGE_MEMORY)
	  set(PSP_LARGE_MEMORY_FLAGS -d MEMSIZE=1)
	endif()

	get_target_property(PSP_EXECUTABLE_OUTPUT_NAME ${nm} OUTPUT_NAME)
	if (NOT PSP_EXECUTABLE_OUTPUT_NAME)
	  set(PSP_EXECUTABLE_OUTPUT_NAME ${nm})
	endif(NOT PSP_EXECUTABLE_OUTPUT_NAME)
	set_target_properties(
	  ${nm}
	  PROPERTIES LINK_FLAGS "-specs=${PSPSDK_PATH}/lib/prxspecs -Wl,-q,-T${PSPSDK_PATH}/lib/linkfile.prx ${PSPSDK_PATH}/lib/prxexports.o -L${PSPSDK_PATH}/lib"
	  )

	if(NOT PSP_SFO)
	  set(PSP_SFO PARAM.SFO)
	endif()
	if(NOT PSP_ICON)
	  set(PSP_ICON NULL)
	endif()
	if(NOT PSP_ICON1)
	  set(PSP_ICON1 NULL)
	endif()
	if(NOT PSP_UNKPNG)
	  set(PSP_UNKPNG NULL)
	endif()
	if(NOT PSP_PIC1)
	  set(PSP_PIC1 NULL)
	endif()
	if(NOT PSP_SND0)
	  set(PSP_SND0 NULL)
	endif()
	if(NOT PSP_PSAR)
	  set(PSP_PSAR NULL)
	endif()

	add_custom_command(
	  TARGET ${nm}
	  POST_BUILD
	  COMMAND ${PSP_BIN_PATH}/psp-fixup-imports ${PSP_EXECUTABLE_OUTPUT_NAME}
	  COMMAND ${PSP_BIN_PATH}/mksfo ${PSP_LARGE_MEMORY_FLAGS} '${PSP_EXECUTABLE_OUTPUT_NAME}' ${PSP_SFO}
	  COMMAND ${PSP_BIN_PATH}/psp-prxgen ${PSP_EXECUTABLE_OUTPUT_NAME} ${PSP_EXECUTABLE_OUTPUT_NAME}.prx
	  COMMAND ${PSP_BIN_PATH}/pack-pbp EBOOT.PBP ${PSP_SFO} ${PSP_ICON} ${PSP_ICON1} ${PSP_UNKPNG} ${PSP_PIC1} ${PSP_SND0} ${PSP_EXECUTABLE_OUTPUT_NAME}.prx ${PSP_PSAR}
	  )
  endfunction()
endif()
