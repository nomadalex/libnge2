# - Try to find GStreamer
# Once done, this will define
#
#  GSTREAMER_LIBRARIES, the name of the library to link against
#  GSTREAMER_FOUND, if false, do not try to link to SDL mixer
#  GSTREAMER_INCLUDE_DIRS, where to find headers

if(NOT GSTREAMER_FOUND)
  include(PkgUtils)
  findpkg_begin(GSTREAMER)

  prepare_find_pkg_vars(GSTREAMER)
  use_pkgconfig(GSTREAMER_PKGC gstreamer-0.10)
  
  set(GSTREAMER_INCLUDE_DIR ${GSTREAMER_PKGC_INCLUDE_DIRS})
  set(GSTREAMER_LIBRARY_REL ${GSTREAMER_PKGC_LIBRARIES})
  make_library_set(GSTREAMER_LIBRARY)
  
  findpkg_finish(GSTREAMER)
endif()

