set(NET_SRCS
  event.c
  log.c
  nge_net_common.c
  select.c
  )

add_definitions(-DNGE2_NET)
add_our_library(nge_net STATIC ${NET_SRCS})

install(TARGETS nge_net DESTINATION lib)
install(FILES event.h nge_net_common.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
