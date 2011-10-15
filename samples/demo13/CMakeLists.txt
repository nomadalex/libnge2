string(REGEX MATCH "demo(.+)" D_NAME ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(. ${D_NAME}_SRCS)
add_nge_executable(${D_NAME} ${${D_NAME}_SRCS})

add_resource(${D_NAME}_all_res images
  demo2_bg.jpg
  demo2_box.jpg
  demo2_icon1.png
  demo2_icon0.bmp)
add_resource(${D_NAME}_all_res fonts
  GBK14
  ASC14
  simfang.ttf)
add_resource(${D_NAME}_all_res music
  simple1.mp3
  simple3.wav
  test.ogg)

copy_res_files(${D_NAME} ${${D_NAME}_all_res})
