ADD_DEFINITIONS(-std=c++11)

SET(APPSRC
  shell/application.cc
  config/lt_config.cc

  ./etcdv3/proto/kv.pb.cc
  ./etcdv3/proto/auth.pb.cc
  ./etcdv3/proto/rpc.pb.cc
  ./etcdv3/proto/v3lock.pb.cc
  ./etcdv3/proto/rpc.grpc.pb.cc

  ./etcdv3/etcd_client.cc
  ./etcdv3/etcd_watcher.cc
  ./etcdv3/context/call_context.cc
)

include_directories(
  .
  $ENV{HOME}/include
  )

ADD_LIBRARY(ltapp STATIC ${APPSRC})

target_link_libraries(ltapp ltio)
target_include_directories(ltapp PUBLIC
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/thirdparty
  )

add_dependencies(ltapp
  ltio
  )

#ADD_LIBRARY(ltapp_shared SHARED ${APPSRC})
#
#set_target_properties(ltapp_shared PROPERTIES OUTPUT_NAME "ltapp")
## make shared library and static lib exist same time
#set_target_properties(ltapp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(ltapp_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
