include(LogFeature)
include(PkgUtils)
prepare_dep_search(NGE2)

################################################################################
# nge2
set(NGE2_DEP_LIBS)
# Find zlib
find_package(ZLIB)
log_feature(ZLIB_FOUND "zlib" "Simple data compression library" "http://www.zlib.net" REQUIRED)
add_package(ZLIB NGE2_DEP_LIBS)

find_package(JPEG)
log_feature(JPEG_FOUND "libjpeg" "Library for JPEG image compression" "http://sourceforge.net/projects/libjpeg" REQUIRED)
add_package(JPEG NGE2_DEP_LIBS)

find_package(PNG)
log_feature(PNG_FOUND "libpng" "Official PNG reference library" "http://www.libpng.org/pub/png/libpng.html" REQUIRED)
add_package(PNG NGE2_DEP_LIBS)

find_package(GIF)
log_feature(GIF_FOUND "giflib" " A library for processing GIF" "http://sourceforge.net/projects/giflib" REQUIRED)
add_package(GIF NGE2_DEP_LIBS)

# Find FreeType
find_package(Freetype)
log_feature(FREETYPE_FOUND "freetype" "Portable font engine" "http://www.freetype.org" REQUIRED)
add_package(FREETYPE NGE2_DEP_LIBS)

if(WIN32)
  find_package(SDL)
  log_feature(SDL_FOUND "SDL" "Simple DirectMedia Layer" "http://www.libsdl.org/" REQUIRED)
  add_package(SDL NGE2_DEP_LIBS)
endif()

if(WIN32 OR LINUX)
  # Find OpenGL
  find_package(OpenGL)
  log_feature(OPENGL_FOUND "OpenGL" "OpenGL render system" "http://www.opengl.org/" REQUIRED)
  add_package(OPENGL NGE2_DEP_LIBS)
endif()

################################################################################
# coolauido
set(COOLAUDIO_DEP_LIBS)
if(LINUX)
  find_package(GStreamer)
  log_feature(GSTREAMER_FOUND "Gstreamer" "Open source multimedia framework" "http://gstreamer.freedesktop.org/" REQUIRED)
  add_package(GSTREAMER COOLAUDIO_DEP_LIBS)
endif()

################################################################################
# samples deps
# for demo5
find_package(SQLite3)
log_feature(SQLITE3_FOUND "sqlite3" "a self-contained, serverless, zero-configuration, transactional SQL database engine" "http://www.sqlite.org/")

# for demo9
find_package(Unzip)
log_feature(UNZIP_FOUND "unzip" "a unzip library, you can download it from download page." "")

################################################################################
# Tools
find_package(Doxygen)
log_feature(DOXYGEN_FOUND "Doxygen" "Tool for building API documentation" "http://doxygen.org")

# Display results, terminate if anything required is missing
set(MISS_FEATURE_CUSTOM_MSG "-- If you are in Windows, try passing -DNGE2_DEPENDENCIES_DIR=<path to dependencies>")
display_feature_log()

