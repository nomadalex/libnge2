# Use this command to build the PSP port :
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-psp-gcc.cmake .
#
# or for out of source:
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-psp-gcc.cmake ..
#
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
    # execute_process(COMMAND ${psp-config_SCRIPT}
    # 		    ARGS --pspdev-path
    # 		    OUTPUT_VARIABLE PSPDEV_PATH
    # 		    OUTPUT_STRIP_TRAILING_WHITESPACE)
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

# message("PSP_PREFIX is ${PSP_PREFIX}")
# message("PSPSDK_PATH is ${PSPSDK_PATH}")

# set(CMAKE_SYSTEM_INCLUDE_PATH ${PSP_PREFIX}/include)
# set(CMAKE_SYSTEM_LIBRARY_PATH ${PSP_PREFIX}/lib)
# set(CMAKE_SYSTEM_PROGRAM_PATH ${PSP_PREFIX}/bin)

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

set(PSP 1)

# message("In psptoolchain.cmake")
