set(NET_SRCS
  src/net/event.c
  src/net/log.c
  src/net/nge_net_common.c
  src/net/select.c
  )

add_definitions(-DNGE2_NET)
add_our_library(nge_net STATIC ${NET_SRCS})

install(TARGETS nge_net EXPORT nge2 DESTINATION lib)
install(FILES event.h nge_net_common.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
