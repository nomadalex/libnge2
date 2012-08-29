aux_source_directory(hge _HGE_SRCS)

add_addon_srcs(hge
  HAS_SUBDIR_IN_INCLUDE
  SRC_NO_SUBDIR
  SRCS ${HGE_SRCS})
