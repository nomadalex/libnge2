if(PSP)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/psptoolchain.cmake)

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/psptoolchain.cmake)
  set(CMAKE_INSTALL_PREFIX ${PSPSDK_PATH} CACHE STRING
    "The prefix path for install.")
  message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

  set(SHARED FALSE CACHE BOOL
    "Build shared library." FORCE)

  include_directories(SYSTEM ${PSPSDK_PATH}/include)
  link_directories(${PSPSDK_PATH}/lib)

  set(PSP_FW_VERSION 371 CACHE STRING
    "Your psp fireware version.")
  add_definitions(-D_PSP_FW_VERSION=${PSP_FW_VERSION})

  option(PSP_LARGE_MEMORY "Your psp fireware version." off)
  if(PSP_LARGE_MEMORY)
    set(PSP_LARGE_MEMORY_FLAGS -d MEMSIZE=1)
  endif()

  set(PSPSDK_LIBS -lpspdebug -lpspdisplay_driver -lpspsdk)

  option(USE_KERNEL_LIBS "whether use kernel libs." off)
  option(USE_USER_LIBS "whether use user libs." off)

  if(USE_KERNEL_LIBS)
    list(APPEND PSPSDK_LIBS -lpspctrl_driver)
  else()
    list(APPEND PSPSDK_LIBS -lpspge -lpspctrl -lpspnet -lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility -lpspuser)
    if(NOT USE_USER_LIBS)
      list(APPEND PSPSDK_LIBS -lpspkernel)
    endif()
  endif()

  # Define the overridable parameters for EBOOT.PBP
  set(PSP_EBOOT_SFO PARAM.SFO CACHE STRING
    "Define the sfo for EBOOT.PBP.")
  set(PSP_EBOOT_ICON NULL CACHE STRING
    "Define the icon for EBOOT.PBP.")
  set(PSP_EBOOT_ICON1 NULL CACHE STRING
    "Define the icon1 for EBOOT.PBP.")
  set(PSP_EBOOT_UNKPNG NULL CACHE STRING
    "Define the unk png for EBOOT.PBP.")
  set(PSP_EBOOT_PIC1 NULL CACHE STRING
    "Define the pic1 for EBOOT.PBP.")
  set(PSP_EBOOT_SND0 NULL CACHE STRING
    "Define the snd0 for EBOOT.PBP.")
  set(PSP_EBOOT_PSAR NULL CACHE STRING
    "Define the psar for EBOOT.PBP.")
  # mark_as_advanced(PSP_EBOOT_SFO PSP_EBOOT_ICON1 PSP_EBOOT_UNKPNG PSP_EBOOT_SND0 PSP_EBOOT_PSAR)

  # After building the ELF binary build the PSP executable.
  function(pack_psp_eboot nm)
    get_target_property(PSP_EXECUTABLE_OUTPUT_NAME ${nm} OUTPUT_NAME)
    if (NOT PSP_EXECUTABLE_OUTPUT_NAME)
      set(PSP_EXECUTABLE_OUTPUT_NAME ${nm})
    endif(NOT PSP_EXECUTABLE_OUTPUT_NAME)
    set_target_properties(
      ${nm}
      PROPERTIES LINK_FLAGS "-specs=${PSPSDK_PATH}/lib/prxspecs -Wl,-q,-T${PSPSDK_PATH}/lib/linkfile.prx ${PSPSDK_PATH}/lib/prxexports.o -L${PSPSDK_PATH}/lib"
      )
    target_link_libraries(${nm} ${PSPSDK_LIBS} c)
    add_custom_command(
      TARGET ${nm}
      POST_BUILD
      COMMAND psp-fixup-imports ${PSP_EXECUTABLE_OUTPUT_NAME}
      COMMAND mksfo ${PSP_LARGE_MEMORY_FLAGS} '${PSP_EXECUTABLE_OUTPUT_NAME}' ${PSP_EBOOT_SFO}
      COMMAND psp-prxgen ${PSP_EXECUTABLE_OUTPUT_NAME} ${PSP_EXECUTABLE_OUTPUT_NAME}.prx
      COMMAND pack-pbp EBOOT.PBP ${PSP_EBOOT_SFO} ${PSP_EBOOT_ICON} ${PSP_EBOOT_ICON1} ${PSP_EBOOT_UNKPNG} ${PSP_EBOOT_PIC1} ${PSP_EBOOT_SND0} ${PSP_EXECUTABLE_OUTPUT_NAME}.prx ${PSP_EBOOT_PSAR}
      )
  endfunction()
  # message("In Psptools.cmake")
endif()
