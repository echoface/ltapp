ADD_DEFINITIONS(-std=c++11)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

SET(APPSRC
  ./application.cc
)

SET(LTBASE_LINKLIBS
  ltnet
  ${GLOG_LIBRARY}
  ${ZLIB_LIBRARIES}
)

include_directories(
  ${PROJECT_SOURCE_DIR}/app
  ${PROJECT_SOURCE_DIR}/thirdparty/ltio
  ${PROJECT_SOURCE_DIR}/thirdparty/ltio/thirdparty
)

ADD_LIBRARY(ltapp STATIC ${APPSRC})
ADD_LIBRARY(ltapp_shared SHARED ${APPSRC})

set_target_properties(ltapp_shared PROPERTIES OUTPUT_NAME "ltapp")
# make shared library and static lib exist same time
set_target_properties(ltapp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(ltapp_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
