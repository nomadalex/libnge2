string(REGEX MATCH "demo(.+)" D_NAME ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(. ${D_NAME}_SRCS)
add_nge_executable(${D_NAME} ${${D_NAME}_SRCS})

add_resource(${D_NAME}_all_res images
  demo1_bg.png
  demo1_box.jpg
  demo1_icon.jpg)
add_resource(${D_NAME}_all_res fonts
  GBK14
  ASC14
  simfang.ttf)

copy_res_files(${D_NAME} ${${D_NAME}_all_res})
