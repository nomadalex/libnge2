if(PSP)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/psptoolchain.cmake)

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/psptoolchain.cmake)
  set(CMAKE_INSTALL_PREFIX ${PSPSDK_PATH} CACHE STRING
      "The prefix path for install."
      FORCE)
  message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

  set(SHARED FALSE CACHE BOOL
    "Build shared library."
    FORCE)

  include_directories(SYSTEM ${PSPSDK_PATH}/include)
  link_directories(${PSPSDK_PATH}/lib)

  # After building the ELF binary build the PSP executable.
  function(fix_psp_executable nm)
    get_target_property(PSP_EXECUTABLE_OUTPUT_NAME ${nm} OUTPUT_NAME)
    if (NOT PSP_EXECUTABLE_OUTPUT_NAME)
      set(PSP_EXECUTABLE_OUTPUT_NAME ${nm})
    endif(NOT PSP_EXECUTABLE_OUTPUT_NAME)
    set_target_properties(
      ${nm}
      PROPERTIES LINK_FLAGS "-specs=${PSPSDK_PATH}/lib/prxspecs -Wl,-q,-T${PSPSDK_PATH}/lib/linkfile.prx ${PSPSDK_PATH}/lib/prxexports.o -L${PSPSDK_PATH}/lib"
      )
    add_custom_command(
      TARGET ${nm}
      POST_BUILD
      COMMAND psp-fixup-imports ${PSP_EXECUTABLE_OUTPUT_NAME}
      COMMAND mksfo '${PSP_EXECUTABLE_OUTPUT_NAME}' PARAM.SFO
      COMMAND psp-prxgen ${PSP_EXECUTABLE_OUTPUT_NAME} ${PSP_EXECUTABLE_OUTPUT_NAME}.prx
      COMMAND pack-pbp EBOOT.PBP PARAM.SFO NULL NULL NULL NULL NULL ${PSP_EXECUTABLE_OUTPUT_NAME}.prx NULL
      )
  endfunction()
  # message("In Psptools.cmake")
endif()
