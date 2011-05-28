string(REGEX MATCH "demo(.+)" D_NAME ${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(. ${D_NAME}_SRCS)
include_directories(${SQLITE3_INCLUDE_DIR})
add_nge_executable(${D_NAME} ${${D_NAME}_SRCS} LIBRARIES ${SQLITE3_LIBRARIES})

add_resource(${D_NAME}_all_res images
  demo1_bg.jpg
  demo1_box.jpg
  demo1_icon1.png
  demo1_icon0.bmp)
add_resource(${D_NAME}_all_res fonts
  GBK14
  ASC14
  simfang.ttf)
add_resource(${D_NAME}_all_res database
  infodata.db)

copy_res_files(${D_NAME} ${${D_NAME}_all_res})
