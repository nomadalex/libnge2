if(PSP)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/psptoolchain.cmake)

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/psptoolchain.cmake)
  set(CMAKE_INSTALL_PREFIX ${PSPSDK_PATH} CACHE STRING
    "The prefix path for install.")
  message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

  set(SHARED FALSE CACHE BOOL
    "Build shared library." FORCE)

  include_directories(SYSTEM ${PSPSDK_PATH}/include)

  include(CMakeParseArguments)
  # After building the ELF binary build the PSP executable.
  # FW_VERSION "Your psp fireware version."
  # KERNEL "whether use kernel libs."
  # USER "whether use user libs."
  # SFO "Define the sfo for EBOOT.PBP."
  # ICON "Define the icon for EBOOT.PBP."
  # ICON1 "Define the icon1 for EBOOT.PBP."
  # UNKPNG "Define the unk png for EBOOT.PBP."
  # PIC1 "Define the pic1 for EBOOT.PBP."
  # SND0 "Define the snd0 for EBOOT.PBP."
  # PSAR "Define the psar for EBOOT.PBP."
  function(pack_psp_eboot nm)
    set(options LARGE_MEMORY KERNEL USER)
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

  set(PSPSDK_LIBS -lpspdebug -lpspdisplay_driver -lpspsdk)

  if(PSP_KERNEL)
    list(APPEND PSPSDK_LIBS -lpspctrl_driver)
  else()
    list(APPEND PSPSDK_LIBS -lpspge -lpspctrl -lpspnet -lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility -lpspuser)
    if(NOT PSP_USER)
      list(APPEND PSPSDK_LIBS -lpspkernel)
    endif()
  endif()

  # Define the overridable parameters for EBOOT.PBP
    get_target_property(PSP_EXECUTABLE_OUTPUT_NAME ${nm} OUTPUT_NAME)
    if (NOT PSP_EXECUTABLE_OUTPUT_NAME)
      set(PSP_EXECUTABLE_OUTPUT_NAME ${nm})
    endif(NOT PSP_EXECUTABLE_OUTPUT_NAME)
    set_target_properties(
      ${nm}
      PROPERTIES LINK_FLAGS "-specs=${PSPSDK_PATH}/lib/prxspecs -Wl,-q,-T${PSPSDK_PATH}/lib/linkfile.prx ${PSPSDK_PATH}/lib/prxexports.o -L${PSPSDK_PATH}/lib"
      )
    target_link_libraries(${nm} ${PSPSDK_LIBS} c)

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
      COMMAND psp-fixup-imports ${PSP_EXECUTABLE_OUTPUT_NAME}
      COMMAND mksfo ${PSP_LARGE_MEMORY_FLAGS} '${PSP_EXECUTABLE_OUTPUT_NAME}' ${PSP_SFO}
      COMMAND psp-prxgen ${PSP_EXECUTABLE_OUTPUT_NAME} ${PSP_EXECUTABLE_OUTPUT_NAME}.prx
      COMMAND pack-pbp EBOOT.PBP ${PSP_SFO} ${PSP_ICON} ${PSP_ICON1} ${PSP_UNKPNG} ${PSP_PIC1} ${PSP_SND0} ${PSP_EXECUTABLE_OUTPUT_NAME}.prx ${PSP_PSAR}
      )
  endfunction()
  # message("In Psptools.cmake")
endif()
