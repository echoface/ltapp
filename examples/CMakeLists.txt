
add_executable(sample_app
  lt_app/main.cpp
  lt_app/app_context_impl.cc
)

target_link_libraries(sample_app
  ltapp
  ltio
  grpc++
)

add_executable(async_mysql_test
  async_mysql/asyncmysql_test.cc
)
target_link_libraries(async_mysql_test
  ltasyncmysql
)

add_executable(etcdclient_test
  etcdclient/test_client.cc
)

target_link_libraries(etcdclient_test
  ltapp
  ltio
  grpc++
  )

target_include_directories(etcdclient_test
  PUBLIC
  $ENV{HOME}/include
  ${PROJECT_SOURCE_DIR}/app
  ${PROJECT_SOURCE_DIR}/thirdparty
)
