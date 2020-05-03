#ifndef _LT_APP_ETCD_WATCH_H_
#define _LT_APP_ETCD_WATCH_H_

#include "context/call_context.h"
#include "etcd_client.h"
#include "google/protobuf/util/json_util.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"
#include "proto/rpc.pb.h"
#include <memory>
namespace lt {

/*
  watch = NewWatcher(client)

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
  }while(1);
*/

class EtcdWatcher {
public:
  EtcdWatcher(EtcdClientV3* client);

  RefWatchContext Watch(const std::string& key, bool with_prefix = true);
  RefWatchContext Watch(const WatchRequest& request);
private:
  Watch::Stub* watch_stub();
  base::MessageLoop* loop();
  grpc::CompletionQueue* c_queue();
  EtcdClientV3* client_ = nullptr;
};

}
#endif
