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
using etcdserverpb::LeaseKeepAliveResponse;

namespace lt {

using PutContext = ResponseCallContext<PutResponse>;
using RangeCallContext = ResponseCallContext<RangeResponse>;
using LeaseGrantContext = ResponseCallContext<LeaseGrantResponse>;

EtcdClientV3::EtcdClientV3(MessageLoop* io)
  : loop_(io) {
  loop_->PostTask(FROM_HERE,
                  &MessageLoop::InstallPersistRunner,
                  io, (base::PersistRunner*)this);
}

EtcdClientV3::~EtcdClientV3() {

  Finalize();

  if (thread_ && thread_->joinable()) {
    thread_->join();
  }
}

void EtcdClientV3::Initilize(const Options& opt) {

  channel_ = grpc::CreateChannel(opt.addr, grpc::InsecureChannelCredentials());
  CHECK(channel_);

  kv_stub_= KV::NewStub(channel_);
  CHECK(kv_stub_);

  lease_stub_ = Lease::NewStub(channel_);
  CHECK(lease_stub_);

  if (!opt.poll_in_loop) {
    auto func = std::bind(&EtcdClientV3::PollCompleteQueueMain, this);
    thread_.reset(new std::thread(std::move(func)));
  }
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

  etcd_ctx_await_pre(call_context);

  grpc::ClientContext* context = call_context.ClientContext();
  auto reader = kv_stub_->AsyncPut(context, request, &c_queue_);

  reader->Finish(call_context.MutableResponse(),
                 call_context.MutableStatus(),
                 &call_context);

  etcd_ctx_await_end(call_context);

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
  etcd_ctx_await_pre(call_ctx);

  auto reader = kv_stub_->AsyncRange(
    call_ctx.ClientContext(), get_request, &c_queue_);

  reader->Finish(
    call_ctx.MutableResponse(), call_ctx.MutableStatus(), &call_ctx);

  etcd_ctx_await_end(call_ctx);

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

  etcd_ctx_await_pre(call_context);

  auto reader = lease_stub_->AsyncLeaseGrant(call_context.ClientContext(),
                                             request, &c_queue_);
  reader->Finish(call_context.MutableResponse(),
                 call_context.MutableStatus(), &call_context);

  etcd_ctx_await_end(call_context);

  return call_context.GetResponse().id();
}

RefKeepAliveContext EtcdClientV3::LeaseKeepalive(int64_t lease,
                                                 int64_t interval) {
  CHECK(CO_CANYIELD);

  RefKeepAliveContext call_ctx(new KeepAliveContext);

  if (!call_ctx->Initilize(lease_stub_.get(), &c_queue_)) {
    LOG(ERROR) << __FUNCTION__
    << " keeaplive initialize failed, lease id:" << lease;
    return nullptr;
  }

  VLOG(GLOG_VINFO) << __FUNCTION__
    << " keeaplive stream connted, id:" << lease;

  CO_GO base::MessageLoop::Current() << [=]() {
    call_ctx->KeepAliveInternal(lease, interval);
  };
  return call_ctx;
}

void EtcdClientV3::Finalize() {
  c_queue_.Shutdown();
}

void EtcdClientV3::Run() {
  bool ok = false;
  void* got_tag = nullptr;
  bool poll_continue = true;

  do {
    auto deadline =
      std::chrono::system_clock::now() + std::chrono::microseconds(0);

    switch (c_queue_.AsyncNext(&got_tag, &ok, deadline)) {
      case grpc::CompletionQueue::GOT_EVENT: {
        // Verify that the request was completed successfully. Note that "ok"
        // corresponds solely to the request for updates introduced by Finish().
        LOG_IF(ERROR, !ok) << " grpc operation failed for context:" << got_tag;

        // The tag in this example is the memory location of the call object
        CallContext* call_context = static_cast<CallContext*>(got_tag);
        GPR_ASSERT(call_context);

        call_context->ResumeContext(ok);
        poll_continue = true;
      } break;
      case grpc::CompletionQueue::TIMEOUT: {
        poll_continue = false;
        LOG_EVERY_N(INFO, 10000) << __FUNCTION__
          << ", etcd grpc CompletionQueue poll shutdown or timeout";
      } break;
      case grpc::CompletionQueue::SHUTDOWN: {
        poll_continue = false;
        LOG_EVERY_N(INFO, 10000) << __FUNCTION__
          << ", etcd grpc CompletionQueue poll shutdown or timeout";
      } break;
      defaut:
        poll_continue = false;
        break;
    }
  } while(poll_continue);
  VLOG(GLOG_VTRACE) << __func__ << " etcd grpc CompletionQueue poll";
}

void EtcdClientV3::PollCompleteQueueMain() {
  void* got_tag;
  bool ok = false;

  // Block until the next result is available in the completion queue "cq".
  while (c_queue_.Next(&got_tag, &ok)) {

    // Verify that the request was completed successfully. Note that "ok"
    // corresponds solely to the request for updates introduced by Finish().
    LOG_IF(ERROR, !ok) << __FUNCTION__
      << ", grpc operation failed for context:" << got_tag;

    // The tag in this example is the memory location of the call object
    CallContext* call_context = static_cast<CallContext*>(got_tag);
    GPR_ASSERT(call_context);

    call_context->ResumeContext(ok);
  }
  VLOG(GLOG_VTRACE) << __func__ << " etcd grpc CompletionQueue poll";
}


}//end lt
