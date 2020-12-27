# top level include dir

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/thirdparty
)

#include(cmake/grpc_deps.cmake)

ADD_SUBDIRECTORY(content)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(integration)
ADD_SUBDIRECTORY(thirdparty/ltio)
