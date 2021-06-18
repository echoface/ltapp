
add_executable(simple_app
  lt_app/main.cpp
  lt_app/app_context_impl.cc
)

target_link_libraries(simple_app
  ltapp
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
  )

target_include_directories(etcdclient_test
  PUBLIC ${PROJECT_SOURCE_DIR}/app
)
