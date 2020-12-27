#include "watch_context.h"

namespace lt {

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
    LOG(INFO) << "failed created a watcher";
    return false;
  }

  watch_id_ = response_.watch_id();
  LOG(INFO) << "watcher created, watch id:" << watch_id_;
  return watch_id_ >= 0;
}

bool WatchContext::WaitEvent(WatchResponse* response) {
  CHECK(CO_CANYIELD);

  etcd_ctx_await_pre(*this);
  stream_->Read(&response_, this);
  etcd_ctx_await_end(*this);
  if (Success()) {
    response->CopyFrom(response_);
  }
  return success_;
}

// this all is safe for calling in anywhere
void WatchContext::WaitEvent(WatchEventFunc handler) {
  CO_GO loop_ << [&]() {WaitInternal(std::move(handler));};
}

void WatchContext::WaitInternal(WatchEventFunc handler) {
  CHECK(CO_CANYIELD);

  do {
    etcd_ctx_await_pre(*this);
    stream_->Read(&response_, this);
    etcd_ctx_await_end(*this);

    if (!Success()) {
      LOG(ERROR) << __FUNCTION__ << " wait event";
      continue;
    }

    for (const auto& event : response_.events()) {
      handler(event);
    }
  }while(!IsCanceled());
}

}//end lt
