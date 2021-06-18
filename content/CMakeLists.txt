SET(CONTENT_SRC
  shell/application.cc
  shell/app_context.cc
  config/lt_config.cc

  etcdv3/etcd_client.cc
  etcdv3/etcd_watcher.cc
  etcdv3/context/call_context.cc
  etcdv3/context/keepalive_ctx.cc
  etcdv3/context/watch_context.cc

  registry/etcd_registry.cc
)

set(GRPC_PROTO_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/etcdv3/proto/rpc.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/etcdv3/proto/v3lock.proto
  )
set(PROTO_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/etcdv3/proto/kv.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/etcdv3/proto/auth.proto
  )

set(PROTO_CPP_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/etcdv3/proto)
file(MAKE_DIRECTORY ${PROTO_CPP_OUT_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_CPP_OUT_DIR} ${PROTO_FILES})
# grpc_generate_cpp will generate pb.cc && grpc.pb.cc files
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_CPP_OUT_DIR} ${GRPC_PROTO_FILES})

list(APPEND CONTENT_SRC ${PROTO_SRCS} ${GRPC_SRCS})

ADD_LIBRARY(ltapp STATIC ${CONTENT_SRC})

add_dependencies(ltapp ltio)
target_link_libraries(ltapp
  PUBLIC ltio
  PUBLIC gRPC::grpc++
  )
target_include_directories(ltapp
  PUBLIC ./
  PUBLIC ${PROTO_CPP_OUT_DIR}
  PUBLIC ${PROJECT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
  PUBLIC ${PROJECT_SOURCE_DIR}/thirdparty
)

#ADD_LIBRARY(ltapp_shared SHARED ${APPSRC})
#
#set_target_properties(ltapp_shared PROPERTIES OUTPUT_NAME "ltapp")
## make shared library and static lib exist same time
#set_target_properties(ltapp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(ltapp_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
