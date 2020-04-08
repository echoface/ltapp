#include "application.h"
#include <gflags/gflags.h>
#include <base/message_loop/message_loop.h>

DEFINE_string(config, "manifest.xml", "config file path for tihs app");

namespace lt {

Application::Application(AppContext* context)
  :context_(context){
  main_.SetLoopName(context->Name());
}

Application::~Application() {}

Application& Application::With(const BootupItem& item) {
  bootup_tasks_.push_back(item);
  return *this;
}

void Application::Run() {
  main_.Start();

  for (auto& item : bootup_tasks_) {
    LOG(INFO) << "Start Run Booup Task:" << item.name;
    bool success = false;
    switch(item.exc_type) {
      case ExcRunType::kSequenceType:
        success = item.task();
        break;
      case ExcRunType::kAsyncRunType:
        break;
      case ExcRunType::kAsyncCoroType:
        break;
      default:
        break;
    }
  }
  main_.WaitLoopEnd();
}

} // namespace lt
