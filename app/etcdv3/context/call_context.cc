
#include "call_context.h"
#include "base/coroutine/coroutine_runner.h"
#include <condition_variable>
#include <mutex>

namespace lt {

void CallContext::ResumeContext(bool ok) {
  success_ = ok;
  if (resumer_) {
    LOG(INFO) << "context@" << this << " going to resume";
    return resumer_();
  }
  LOG(ERROR) << "context@" << this << " no resumer locked";
}

void CallContext::LockContext() {
  success_ = false;
  resumer_ = co_resumer();
}

WatchContext::WatchContext(Watch::Stub* stub, base::MessageLoop* loop)
  : watch_id_(-1),
    stub_(stub),
    loop_(loop) {
}

bool WatchContext::InitWithQueue(const WatchRequest& request,
                                 grpc::CompletionQueue* c_queue) {

  etcd_ctx_await_pre(*this);
  stream_ = stub_->AsyncWatch(&context_, c_queue, this);
  etcd_ctx_await_end(*this);
  VLOG(GLOG_VINFO) << "watch stream connted";

  etcd_ctx_await_pre(*this);
  stream_->Write(request, this);
  etcd_ctx_await_end(*this);
  VLOG(GLOG_VINFO) << "write watch request done";

  etcd_ctx_await_pre(*this);
  stream_->Read(&response_, this);
  etcd_ctx_await_end(*this);

  if (!response_.created()) {
    LOG(INFO) << "watch context:" << this << " failed created a watcher";
    return false;
  }
  watch_id_ = response_.watch_id();
  LOG(INFO) << "create watch success, watch id:" << watch_id_;
  return watch_id_ >= 0;
}

bool WatchContext::WaitEvent(WatchResponse* response) {
  etcd_ctx_await_pre(*this);
  stream_->Read(&response_, this);
  etcd_ctx_await_end(*this);

  response->CopyFrom(response_);
  return success_;
}

// this all is safe for calling in anywhere
void WatchContext::WaitEvent(WatchEventFunc handler) {
  co_go loop_ << std::bind(&WatchContext::wait_event_internal, this, handler);
}

void WatchContext::wait_event_internal(WatchEventFunc handler) {
  CHECK(co_can_yield);

  do {
    etcd_ctx_await_pre(*this);
    stream_->Read(&response_, this);
    etcd_ctx_await_end(*this);

    if (!success_) {
      continue;
    }
    for (const auto& event : response_.events()) {
      handler(event);
    }
  }while(1);
}

}
