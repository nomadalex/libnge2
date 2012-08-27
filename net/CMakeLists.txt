set(NET_SRCS
  event.c
  log.c
  nge_net_common.c
  select.c
  )

add_definitions(-DNGE2_NET)
add_addon_srcs(net
  INCLUDE_THIS_DIR
  SRCS ${NET_SRCS})
