SET(APP_SRC
  shell/application.cc
  shell/app_context.cc
  config/lt_config.cc

  etcdv3/proto/kv.pb.cc
  etcdv3/proto/auth.pb.cc
  etcdv3/proto/rpc.pb.cc
  etcdv3/proto/v3lock.pb.cc
  etcdv3/proto/rpc.grpc.pb.cc
  etcdv3/etcd_client.cc
  etcdv3/etcd_watcher.cc
  etcdv3/context/call_context.cc
  etcdv3/context/keepalive_ctx.cc
  etcdv3/context/watch_context.cc

  registry/etcd_registry.cc
)

ADD_LIBRARY(ltapp STATIC ${APP_SRC})

add_dependencies(ltapp ltio)
target_link_libraries(ltapp ltio)
target_include_directories(ltapp
  PUBLIC ./
  PUBLIC $ENV{HOME}/include
  PUBLIC ${PROJECT_SOURCE_DIR}
  PUBLIC ${PROJECT_SOURCE_DIR}/thirdparty
)


#ADD_LIBRARY(ltapp_shared SHARED ${APPSRC})
#
#set_target_properties(ltapp_shared PROPERTIES OUTPUT_NAME "ltapp")
## make shared library and static lib exist same time
#set_target_properties(ltapp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(ltapp_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
