
#include "call_context.h"
#include <condition_variable>
#include <mutex>

namespace lt {

void CallContext::ResumeContext(bool ok) {
  VLOG(GLOG_VINFO) << "context@" << this << " resume start";
  success_ = ok;
  done_.store(true);
  mu_.lock();
  if (resumer_) {
    resumer_();
  }
  mu_.unlock();
}

bool CallContext::LockContext() {
  CHECK(CO_CANYIELD);

  mu_.lock();
  resumer_ = std::move(CO_RESUMER);
  mu_.unlock();
  return done_.load();
}

void CallContext::Reset() {
    success_ = false;
    done_.store(false);
    cancel_.store(false);
    resumer_ = nullptr;
}

}
