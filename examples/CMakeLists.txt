
add_executable(sample_app
  lt_app/main.cpp
  lt_app/app_context_impl.cc
  )

target_link_libraries(sample_app
  ltapp
  ltio
  grpc++
  )

target_include_directories(sample_app
  PUBLIC
  $ENV{HOME}/include
  ${PROJECT_SOURCE_DIR}/app
  ${PROJECT_SOURCE_DIR}/thirdparty
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
