
add_executable(sample_app
  fw_app/main.cpp
  fw_app/fw_context.cpp
  )

target_link_libraries(sample_app
  ltapp
  ltio
  lt3rd
  glog::glog
  gflags::gflags
  pthread
  )

target_include_directories(sample_app
  PUBLIC
  ${PROJECT_SOURCE_DIR}/app
  ${PROJECT_SOURCE_DIR}/thirdparty
  )
