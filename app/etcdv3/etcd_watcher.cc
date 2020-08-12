
#include "etcd_watcher.h"
#include "grpc/grpc.h"
#include "proto/rpc.pb.h"
#include "context/call_context.h"
#include <base/coroutine/coroutine_runner.h>

namespace lt {

EtcdWatcher::EtcdWatcher(EtcdClientV3* client)
  : client_(client) {
}

//create and return a context for recieve DEL/PUT Notify
RefWatchContext EtcdWatcher::Watch(const std::string& key, bool with_prefix) {

  WatchRequest watch_req;

  auto create_req = watch_req.mutable_create_request();

  create_req->set_key(key);
  create_req->set_prev_kv(true);
  create_req->set_start_revision(0);

  if (with_prefix) {
    std::string range_end(key);
    int ascii = (int)range_end[range_end.length()-1];
    range_end.back() = ascii+1;
    create_req->set_range_end(range_end);
  }
  return Watch(watch_req);
}

RefWatchContext EtcdWatcher::Watch(const WatchRequest& request) {
  CHECK(co_can_yield);

  Watch::Stub* stub = watch_stub();
  RefWatchContext ctx(new WatchContext(stub, loop()));

  if (!ctx->InitWithQueue(request, c_queue())) {
    LOG(ERROR) << __func__ << " init watcher failed";
    return nullptr;
  }
  return ctx;
}

Watch::Stub* EtcdWatcher::watch_stub() {
  return client_->watch_stub_.get();
}

base::MessageLoop* EtcdWatcher::loop() {
  return client_->loop_;
}

grpc::CompletionQueue* EtcdWatcher::c_queue() {
  return &client_->c_queue_;
}

}
