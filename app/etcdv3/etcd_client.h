#ifndef _LTAPP_ETCD_CLIENT_H_H_
#define _LTAPP_ETCD_CLIENT_H_H_

#include <memory>
#include <thread>
#include <bits/stdint-intn.h>
#include <vector>

#include "proto/kv.pb.h"
#include "proto/rpc.pb.h"
#include "proto/rpc.grpc.pb.h"
#include "grpcpp/grpcpp.h"
#include "google/protobuf/message.h"
#include "grpcpp/impl/codegen/status.h"
#include "grpcpp/impl/codegen/config_protobuf.h"

#include "context/call_context.h"
#include "context/keepalive_ctx.h"

namespace base {
  class MessageLoop;
}
using base::MessageLoop;
using etcdserverpb::KV;
using etcdserverpb::Watch;
using etcdserverpb::Lease;
using grpc::protobuf::Message;

using namespace mvccpb;
using namespace etcdserverpb;
typedef std::vector<mvccpb::KeyValue> KeyValues;


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

  //success return a reversion id, return -1 when failed
  int64_t Put(const KeyValue& kvs);
  int64_t Put(const PutRequest& request);

  int64_t LeaseGrant(int ttl);

  KeyValues Range(const std::string& key, bool with_prefix = true);

  /*this keepalive lease in background and return a context use for cancelling*/
  RefKeepAliveContext LeaseKeepalive(int64_t lease, int64_t interval = 1000);

private:
  friend class EtcdWatcher;

  //override from base::PersistRunner
  void Sched();

  void PollCompleteQueueMain();

  base::MessageLoop* loop_;
  CompletionQueue c_queue_;
  std::unique_ptr<KV::Stub> kv_stub_;
  std::unique_ptr<Watch::Stub> watch_stub_;
  std::unique_ptr<Lease::Stub> lease_stub_;
  std::unique_ptr<std::thread> thread_;
};

}
#endif
