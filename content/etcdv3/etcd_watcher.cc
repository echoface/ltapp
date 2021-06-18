#include "etcd_watcher.h"

#include <base/coroutine/coroutine_runner.h>
#include "context/call_context.h"
#include "etcdv3/proto/rpc.grpc.pb.h"

namespace lt {

//static
EtcdWatcher::EtcdWatcher(EtcdClientV3* client)
  : client_(client) {
  stub_ = Watch::NewStub(client_->Channel());
  CHECK(stub_);
}

//create and return a context for recieve DEL/PUT Notify
RefWatchContext EtcdWatcher::Watch(const std::string &key,
                                   const bool with_prefix,
                                   const WatchEventFunc& callback) {

  WatchRequest watch_req;

  auto create_req = watch_req.mutable_create_request();

  create_req->set_key(key);
  create_req->set_prev_kv(true);
  create_req->set_start_revision(0);
  create_req->set_watch_id(1);

  if (with_prefix) {
    std::string range_end(key);
    int ascii = (int)range_end[range_end.length()-1];
    range_end.back() = ascii+1;
    create_req->set_range_end(range_end);
  }
  return Watch(watch_req, callback);
}

RefWatchContext EtcdWatcher::Watch(const WatchRequest& request,
                                   const WatchEventFunc& callback) {
  CHECK(CO_CANYIELD);

  RefWatchContext ctx(new WatchContext(stub_.get(), loop()));

  if (!ctx->Start(request, callback)) {
    LOG(ERROR) << __func__ << " init watcher failed";
    return nullptr;
  }
  return ctx;
}

base::MessageLoop* EtcdWatcher::loop() {
  return client_->GetLoop();
}

} //end namespace lt
