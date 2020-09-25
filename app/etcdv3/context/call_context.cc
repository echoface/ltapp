
#include "call_context.h"
#include <condition_variable>
#include <mutex>

namespace lt {

void CallContext::ResumeContext(bool ok) {
  success_ = ok;
  if (resumer_) {
    return resumer_();
  }
  LOG(ERROR) << "context@" << this << " no resumer locked";
}

void CallContext::LockContext() {
  success_ = false;
  resumer_ = co_resumer();
}

}
