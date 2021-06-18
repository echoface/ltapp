#include "etcd_client.h"

#include "base/coroutine/coroutine_runner.h"
#include "base/message_loop/message_loop.h"
#include "context/call_context.h"
#include "etcdv3/proto/kv.pb.h"
#include "etcdv3/proto/rpc.pb.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"

using etcdserverpb::PutRequest;
using etcdserverpb::PutResponse;
using etcdserverpb::RangeRequest;
using etcdserverpb::RangeResponse;
using etcdserverpb::LeaseGrantResponse;
using etcdserverpb::LeaseTimeToLiveRequest;

namespace lt {

using PutContext = ResponseCallContext<PutResponse>;
using RangeCallContext = ResponseCallContext<RangeResponse>;
using LeaseGrantContext = ResponseCallContext<LeaseGrantResponse>;
using LeaseTimeToLiveCtx = ResponseCallContext<etcdserverpb::LeaseTimeToLiveResponse>;

EtcdClientV3::EtcdClientV3(MessageLoop* io)
  : loop_(io) {
}

EtcdClientV3::~EtcdClientV3() {
  Finalize();
}

void EtcdClientV3::Initilize(const Options& opt) {

  channel_ = grpc::CreateChannel(opt.addr, grpc::InsecureChannelCredentials());
  CHECK(channel_);

  kv_stub_= KV::NewStub(channel_);
  CHECK(kv_stub_);

  lease_stub_ = Lease::NewStub(channel_);
  CHECK(lease_stub_);
}


int64_t EtcdClientV3::Put(const KeyValue& kvs) {
  PutRequest request;

  request.set_key(kvs.key());
  request.set_value(kvs.value());
  request.set_lease(kvs.lease());

  return Put(request);
}

int64_t EtcdClientV3::Put(const PutRequest& request) {
  CHECK(CO_CANYIELD);

  PutContext call_context;

  call_context.LockContext();
  grpc::ClientContext *context = call_context.ClientContext();
  kv_stub_->async()->Put(context, &request, call_context.MutableResponse(),
                         [&](grpc::Status st) {
                           *call_context.MutableStatus() = st;
                           call_context.ResumeContext(st.ok());
                         });
  CO_YIELD;

  if (call_context.IsStatusOK()) {
    return call_context.GetResponse().header().revision();
  }

  LOG(WARNING) << __FUNCTION__ << ", etcdclient put failed,"
    << "key:" << request.key() << ", value:"  << request.value();
  return -1;
}

KeyValues EtcdClientV3::Range(const std::string& key, bool with_prefix) {
  CHECK(CO_CANYIELD);

  RangeRequest get_request;

  get_request.set_key(key);
  if (with_prefix) {
    std::string range_end(key);
    int ascii = (int)range_end[range_end.length()-1];
    range_end.back() = ascii+1;
    get_request.set_range_end(range_end);
    get_request.set_sort_target(
      RangeRequest::SortTarget::RangeRequest_SortTarget_KEY);
    get_request.set_sort_order(
      RangeRequest::SortOrder::RangeRequest_SortOrder_ASCEND);
  }

  RangeCallContext call_ctx;
  call_ctx.LockContext();
  kv_stub_->async()->Range(call_ctx.ClientContext(), &get_request,
                           call_ctx.MutableResponse(),
                           [&](grpc::Status status) {
                              call_ctx.ResumeContext(status.ok());
                           });
  CO_YIELD;

  if (!call_ctx.IsStatusOK()) {
    LOG(WARNING) << __FUNCTION__ << ", etcdclient range failed"
      << "key:" << key << ", with_prefix:" << (with_prefix ? "True" : "False");
    return KeyValues();
  }

  KeyValues kvs;
  for (auto& kv : call_ctx.GetResponse().kvs()) {
    kvs.emplace_back(kv);
  }
  return kvs;
}

int64_t EtcdClientV3::LeaseGrant(int ttl) {
  CHECK(CO_CANYIELD);

  etcdserverpb::LeaseGrantRequest request;
  request.set_ttl(ttl);

  LeaseGrantContext call_context;

  call_context.LockContext();
  lease_stub_->async()->LeaseGrant(call_context.ClientContext(), &request,
                                   call_context.MutableResponse(),
                                   [&](grpc::Status status) {
                                     call_context.ResumeContext(status.ok());
                                   });

  CO_YIELD;
  return call_context.GetResponse().id();
}

RefKeepAliveContext EtcdClientV3::LeaseKeepalive(int64_t lease,
                                                 int64_t interval) {
  CHECK(CO_CANYIELD);
  auto ctx = KeepAliveContext::New(loop_, lease_stub_.get());
  CHECK(ctx->Start(lease, interval));
  return ctx;
}

int64_t EtcdClientV3::TimeToAlive(int64_t lease_id) {
  CHECK(CO_CANYIELD);

  LeaseTimeToLiveCtx call_ctx;
  LeaseTimeToLiveRequest request;
  request.set_id(lease_id);
  lease_stub_->async()->LeaseTimeToLive(
      call_ctx.ClientContext(), &request, call_ctx.MutableResponse(),
      [&](grpc::Status status) {
        call_ctx.ResumeContext(status.ok());
      });

  CO_YIELD; 
  return call_ctx.GetResponse().ttl();
}

void EtcdClientV3::Finalize() {
}

}//end lt
