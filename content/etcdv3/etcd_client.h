#ifndef _LTAPP_ETCD_CLIENT_H_H_
#define _LTAPP_ETCD_CLIENT_H_H_

#include <memory>
#include <thread>
#include <vector>

#include "context/call_context.h"
#include "context/keepalive_ctx.h"
#include "etcdv3/proto/rpc.grpc.pb.h"
#include "etcdv3/proto/v3lock.grpc.pb.h"
#include "google/protobuf/message.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/impl/codegen/config_protobuf.h"
#include "grpcpp/impl/codegen/status.h"

namespace base {
  class MessageLoop;
}
using base::MessageLoop;
using etcdserverpb::KV;
using etcdserverpb::Watch;
using etcdserverpb::Lease;
using etcdserverpb::PutRequest;
using grpc::protobuf::Message;

using KeyValues = std::vector<mvccpb::KeyValue>;

namespace lt {

class EtcdWatcher;

class EtcdClientV3 : base::PersistRunner {
public:
  struct Options {
    std::string addr;
    bool poll_in_loop = true;
  };

  EtcdClientV3(MessageLoop* io);

  ~EtcdClientV3();

  void Initilize(const Options& opt);

  void Finalize();

  // all etcd option run in corotuine context
  //success return a reversion id, return -1 when failed
  __CO_WAIT__
  int64_t Put(const KeyValue& kvs);

  __CO_WAIT__
  int64_t Put(const PutRequest& request);

  __CO_WAIT__
  int64_t LeaseGrant(int ttl);

  __CO_WAIT__
  KeyValues Range(const std::string& key, bool with_prefix = true);

  /*this keepalive lease in background and return a context use for cancelling*/
  __CO_WAIT__
  RefKeepAliveContext LeaseKeepalive(int64_t lease, int64_t interval = 1000);

private:
  friend class EtcdWatcher;

  //override from base::PersistRunner
  void Sched();

  void PollCompleteQueueMain();

  base::MessageLoop* GetLoop() {return loop_;};

  std::shared_ptr<grpc::Channel> Channel() {
    return channel_;
  }

  CompletionQueue* GetCompletionQueue() {
    return &c_queue_;
  }

private:
  base::MessageLoop* loop_;

  CompletionQueue c_queue_;

  std::unique_ptr<KV::Stub> kv_stub_;

  std::unique_ptr<Lease::Stub> lease_stub_;

  std::shared_ptr<grpc::Channel> channel_;

  std::unique_ptr<std::thread> thread_;
};

} //end namespace lt
#endif
