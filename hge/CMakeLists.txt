aux_source_directory(hge HGE_SRCS)

add_addon_srcs(hge
  HAS_SUBDIR_IN_INCLUDE
  SRCS ${HGE_SRCS})
