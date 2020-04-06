ADD_DEFINITIONS(-std=c++11)

SET(APPSRC
  application.cc
  config_loader.cc
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
