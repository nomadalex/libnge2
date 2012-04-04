## Author: Kun Wang <ifreedom.cn@gmail.com>
## Version: $Id: Android.cmake,v 0.0 2012/04/04 06:07:14 ifreedom Exp $

##
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##

function(set_output_root dir)
  set(LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_BINARY_DIR}/Android CACHE PATH "root for library output, set this to change where android libs are installed to" FORCE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/jni/CMakeLists.txt")
	set(EXECUTABLE_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH_ROOT}/bin/${ANDROID_NDK_ABI_NAME}" CACHE PATH "Output directory for applications" FORCE)
  else()
	set(EXECUTABLE_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH_ROOT}/bin" CACHE PATH "Output directory for applications" FORCE)
  endif()
  set(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME}" CACHE PATH "path for android libs" FORCE)
endfunction()

### Android.cmake ends here
