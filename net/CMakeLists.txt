set(NET_SRCS
  event.c
  log.c
  nge_net_common.c
  select.c
  )

add_definitions(-DNGE2_NET)
add_addon_srcs(NGE2_SRCS NET_SRCS net)

install(FILES net/event.h net/nge_net_common.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
