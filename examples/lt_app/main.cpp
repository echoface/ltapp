#include <functional>
#include "glog/logging.h"
#include "gflags/gflags.h"
#include "app_context_impl.h"
#include "content/shell/application.h"
#include "thirdparty/tinyxml2/tinyxml2.h"

DECLARE_string(config);

using lt::BootTask;
using fw::AppContextImpl;
int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  fw::AppContextImpl impl;
  lt::Application app(&impl);

  app
    .Prepare(BootTask::New("config",
                           std::bind(&AppContextImpl::InitConfig, &impl)))
    .Prepare(BootTask::New("client",
                           std::bind(&AppContextImpl::InitClient, &impl)))
    .Prepare(BootTask::New("coro",
                           std::bind(&AppContextImpl::InitCoroPrepare, &impl),
                           lt::ExcRunType::kAsyncCoroType))
    .Prepare(BootTask::New("async",
                           std::bind(&AppContextImpl::InitAsyncPrepare, &impl),
                           lt::ExcRunType::kAsyncRunType))
    .Cleanup(BootTask::New("clean kAsyncCoroType",
                           []() -> bool {
                             LOG(INFO) << "clean kAsyncCoroType run";
                             return true;
                           },
                           lt::ExcRunType::kAsyncCoroType))
    .Cleanup(BootTask::New("clean kSequenceType",
                           []() -> bool {
                             LOG(INFO) << "clean kSequenceType run";
                             return false;
                           }))
    .Cleanup(BootTask::New("clean kAsyncRunType",
                           []() -> bool {
                             LOG(INFO) << "clean kAsyncRunType run";
                             return true;
                           },
                           lt::ExcRunType::kAsyncRunType
                           ))
    .Run();
  return 0;
}
