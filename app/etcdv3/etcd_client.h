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

#include "base/message_loop/message_loop.h"

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
class EtcdClientV3 {
public:
  struct Options {
    std::string addr;
  };


  EtcdClientV3(MessageLoop* io);
  ~EtcdClientV3();

  //success return a reversion id, return -1 when failed
  int64_t Put(const KeyValue& kvs);
  int64_t Put(const PutRequest& request);

  KeyValues Range(const std::string& key,
                  bool with_prefix = true);

  int64_t LeaseGrant(int ttl);

  /*this call not back utill error or revoke a lease*/
  bool LeaseKeepalive(int64_t lease, int64_t interval = 1000);

  void Initilize(const Options& opt);
private:
  friend class EtcdWatcher;
  void PollCompleteQueueMain();

  base::MessageLoop* loop_;
  std::unique_ptr<KV::Stub> kv_stub_;
  std::unique_ptr<Watch::Stub> watch_stub_;
  std::unique_ptr<Lease::Stub> lease_stub_;
  std::unique_ptr<std::thread> thread_;
  grpc::CompletionQueue c_queue_;
};

}
#endif
