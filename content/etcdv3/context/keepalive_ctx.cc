
#include <functional>

#include "base/coroutine/wait_group.h"

#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"

#include "keepalive_ctx.h"

namespace lt {

//this
//static
RefKeepAliveContext KeepAliveContext::New(base::MessageLoop *loop,
                                          Lease::Stub *stub) {
  return RefKeepAliveContext(new KeepAliveContext(loop, stub));
}

KeepAliveContext::KeepAliveContext(base::MessageLoop *l, Lease::Stub *stub)
    : stub_(stub), loop_(l) {
  CHECK(l);
}

bool KeepAliveContext::Start(int64_t lease_id, int64_t interval) {

  client_ctx_.~ClientContext();
  new (&client_ctx_) grpc::ClientContext();

  lease_id_ = lease_id;
  interval_ = interval;

  stub_->async()->LeaseKeepAlive(&client_ctx_, this);
  StartCall();

  request_.set_id(lease_id);
  StartWrite(&request_);
  return true;
}

void KeepAliveContext::Wait() {
  if (LockContext() == true) {
    return;
  }
  // false: mean ResumeContext not call before resumer` be set
  // so it's ok, bz of resumer will be sched into current thread
  CO_YIELD;
}

void KeepAliveContext::OnDone(const grpc::Status &s) {
  LOG(INFO) << __FUNCTION__
    << " keepalive rpc done, msg:" << s.error_message();
  ResumeContext(s.ok());
}

void KeepAliveContext::OnReadDone(bool ok) {
  LOG(INFO) << __FUNCTION__ << " callback, ok:" << ok;

  if (!ok) {
    LOG(ERROR) << __FUNCTION__ << ", read response failed";
    ResumeContext(false);
  }

  if (Canceled()) {
    return StartWritesDone();
  }

  // NOTE: enable shared from this
  auto ref_this = shared_from_this();
  auto next = [ref_this, this]() { ref_this->StartWrite(&request_); };
  loop_->PostDelayTask(NewClosure(next), interval_);
}

void KeepAliveContext::OnWriteDone(bool ok) {
  LOG(INFO) << __FUNCTION__ << " callback, ok:" << ok;
  LOG_IF(ERROR, !ok) << __FUNCTION__ << "write request failed";

  ok ? StartRead(&response_) : ResumeContext(false);
}

void KeepAliveContext::OnWritesDoneDone(bool ok) {
  LOG(INFO) << __FUNCTION__ << " callback, ok:" << ok;
}

}//end lt
