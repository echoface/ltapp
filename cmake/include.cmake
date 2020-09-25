# top level include dir

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/thirdparty
)

ADD_SUBDIRECTORY(app)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(integration)
ADD_SUBDIRECTORY(thirdparty/ltio)
ADD_SUBDIRECTORY(thirdparty/grpc)
