#include <functional>
#include "fw_context.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
#include "content//application.h"
#include "thirdparty/tinyxml2/tinyxml2.h"

DECLARE_string(config);
//DEFINE_string(config, "manifest.xml", "config file path for tihs app");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  FWContext context;
  lt::Application app(&context);

  app.With(lt::WithXMLConfig(FLAGS_config,
                            std::bind(&FWContext::OnXmlConfigLoad, &context, std::placeholders::_1))
           );

  LOG(INFO) << FLAGS_config;
  app.Run();
  return 0;
}
