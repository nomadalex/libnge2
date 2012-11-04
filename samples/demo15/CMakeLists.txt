string(REGEX MATCH "demo(.+)" D_NAME ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(. ${D_NAME}_SRCS)
add_nge_executable(${D_NAME} ${${D_NAME}_SRCS})

add_resource(${D_NAME}_all_res par
  particles.png
  particle1.psi)

copy_res_files(${D_NAME} ${${D_NAME}_all_res})
