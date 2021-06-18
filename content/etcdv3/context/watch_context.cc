#include "watch_context.h"

namespace lt {

WatchContext::WatchContext(Watch::Stub *stub, base::MessageLoop *loop)
    : watch_id_(-1), stub_(stub), loop_(loop) {
}

bool WatchContext::Start(const WatchRequest& request,
                         const WatchEventFunc& callback) {
  CHECK(CO_CANYIELD);
  handler_ = callback;

  stub_->async()->Watch(&client_ctx_, this);

  StartCall();

  write_ctx_.LockContext();
  StartWrite(&request);
  CO_YIELD;

  if (!write_ctx_.Success()) {
    LOG(INFO) << __FUNCTION__ << " send create request fail";
    return false;
  }

  WatchResponse response;
  read_ctx_.LockContext();
  StartRead(&response);
  CO_YIELD;
  if (!read_ctx_.Success()) {
    LOG(INFO) << __FUNCTION__ << " read create watch response fail";
    return false;
  }

  CHECK(response.created());
  watch_id_ = response.watch_id();

  auto refctx = shared_from_this();
  CO_GO std::bind(&WatchContext::watch_change_loop, refctx);
  LOG(INFO) << __FUNCTION__ << " watcher created, id:" << watch_id_;
  return watch_id_ >= 0;
}

void WatchContext::watch_change_loop() {
  LOG(INFO) << __FUNCTION__ << " enter";
  CHECK(CO_CANYIELD);

  auto refctx = shared_from_this();
  WatchResponse response;
  do {
    read_ctx_.LockContext();
    StartRead(&response);
    CO_YIELD;

    if (!read_ctx_.Success()) {
      LOG(INFO) << __FUNCTION__ << " read bad response";
      break;
    }
    if (response.canceled()) {
      LOG(INFO) << __FUNCTION__ << " watcher canceled, reason:"
                << response.cancel_reason();
      break;
    }

    CO_GO [response, refctx]() { // a copy of response
      for (auto& change : response.events()) {
        refctx->handler_(change);
      }
    };
  } while (!read_ctx_.Canceled());

  LOG(INFO) << __FUNCTION__ << " done";
}

bool WatchContext::StopWatch() {
  LOG(INFO) << __FUNCTION__ << " enter";

  CHECK(CO_CANYIELD);
  if (watch_id_ <= 0) {
    return true;
  }

  WatchRequest request;
  auto cancel = request.mutable_cancel_request();
  cancel->set_watch_id(watch_id_);

  write_ctx_.LockContext();
  StartWrite(&request);
  CO_YIELD;
  if (!write_ctx_.Success()) {
    return false;
  }

  read_ctx_.Cancel();

  write_ctx_.LockContext();
  StartWritesDone();
  CO_YIELD;
  if (!write_ctx_.Success()) {
    return false;
  }
  LOG(INFO) << __FUNCTION__ << " done";
  return true;
}

void WatchContext::OnReadDone(bool ok) {
  LOG(INFO) << __FUNCTION__ << " callback, ok:" << ok;
  LOG_IF(ERROR, !ok) << __FUNCTION__ << "read response failed";
  read_ctx_.ResumeContext(ok);
}

void WatchContext::OnWriteDone(bool ok) {
  LOG(INFO) << __FUNCTION__ << " wrinedone rpc done, ok:" << ok;
  LOG_IF(ERROR, !ok) << __FUNCTION__ << "write request fail";
  write_ctx_.ResumeContext(ok);
}

void WatchContext::OnWritesDoneDone(bool ok) {
  LOG(INFO) << __FUNCTION__ << " wrinedone rpc done, ok:" << ok;
  LOG_IF(ERROR, !ok) << __FUNCTION__ << "writedone request fail";
  write_ctx_.ResumeContext(ok);
}

void WatchContext::OnDone(const grpc::Status &s) {
  LOG(INFO) << __FUNCTION__
            << " watch rpc done, msg:" << s.error_message();
  read_ctx_.Cancel();
}

}//end lt
