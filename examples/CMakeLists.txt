
add_executable(sample_app
  fw_app/main.cpp
  fw_app/fw_context.cpp
  )

target_link_libraries(sample_app
  ltapp
  ltio
  )

target_include_directories(sample_app
  PUBLIC
  ${PROJECT_SOURCE_DIR}/app
  ${PROJECT_SOURCE_DIR}/thirdparty
  )

add_executable(test_app
  main.cc
  )

target_link_libraries(test_app
  ltapp
  ltio
  )

target_include_directories(test_app
  PUBLIC
  ${PROJECT_SOURCE_DIR}/app
  ${PROJECT_SOURCE_DIR}/thirdparty
  )

add_executable(async_mysql_test
  async_mysql/asyncmysql_test.cc
  )
target_link_libraries(async_mysql_test
  ltasyncmysql 
  )

