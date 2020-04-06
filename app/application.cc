#include "application.h"
#include <gflags/gflags.h>
#include <base/message_loop/message_loop.h>

namespace lt {

DEFINE_string(config, "manifest.xml", "config file path for tihs app");

LtApplication::LtApplication(AppContext* context) {}

LtApplication::~LtApplication() {}

LtApplication& LtApplication::With(const BootupItem& item) {
  bootup_tasks_.push_back(item);
  return *this;
}

} // namespace lt
