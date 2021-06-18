#ifndef _LT_APP_ETCD_WATCH_H_
#define _LT_APP_ETCD_WATCH_H_

#include <memory>

#include "google/protobuf/util/json_util.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"

#include "context/watch_context.h"
#include "etcdv3/etcd_client.h"
#include "etcdv3/proto/rpc.pb.h"

namespace lt {
/*
  auto watch = NewWatcher(client)

  //follow need call @ coroutine context
  watch_ctx = watch.Watch("/abc", prefix)
  do {
    ev = watch_ctx.WaitEvent();
    switch(ev.type) {
    case PUT: {
    } break;
    case DELETE: {
    } break;
    default:
    break;
    }
  } while(1);

  or

  watch_ctx.WaitEvent([&](event) {
    switch(ev.type) {
    case PUT: {
    } break;
    case DELETE: {
    } break;
    default:
    break;
    }
  });
*/

class EtcdWatcher {
public:
  EtcdWatcher(EtcdClientV3 *client);

  __CO_WAIT__ RefWatchContext Watch(const WatchRequest& request,
                                    const WatchEventFunc& callback);

  __CO_WAIT__ RefWatchContext Watch(const std::string& key,
                                    const bool with_prefix,
                                    const WatchEventFunc& callback);
private:
  base::MessageLoop* loop();

  EtcdClientV3* client_ = nullptr;
  std::unique_ptr<Watch::Stub> stub_;
};

}
#endif
