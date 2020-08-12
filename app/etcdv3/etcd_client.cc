#include "etcd_client.h"
#include "proto/kv.pb.h"
#include "proto/rpc.pb.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"

#include "context/call_context.h"
#include "base/message_loop/message_loop.h"
#include "base/coroutine/coroutine_runner.h"

using etcdserverpb::PutRequest;
using etcdserverpb::LeaseGrantResponse;
using etcdserverpb::LeaseKeepAliveResponse;
namespace lt {

EtcdClientV3::EtcdClientV3(MessageLoop* io)
  : loop_(io) {
  loop_->PostTask(FROM_HERE, &MessageLoop::InstallPersistRunner, io, (base::PersistRunner*)this);
}

EtcdClientV3::~EtcdClientV3() {
  Finalize();
  if (thread_ && thread_->joinable()) {
    thread_->join();
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
  CHECK(co_can_yield);

  typedef ResponseCallContext<PutResponse> PutContext;

  PutContext call_context;

  etcd_ctx_await_pre(call_context);
  auto reader = kv_stub_->AsyncPut(&call_context.context, request, &c_queue_);
  reader->Finish(&call_context.response,&call_context.status, &call_context);
  etcd_ctx_await_end(call_context);

  if (!call_context.IsStatusOK()) {
    return -1;
  }
  return call_context.response.header().revision();
}

KeyValues EtcdClientV3::Range(const std::string& key, bool with_prefix) {
  CHECK(co_can_yield);

  typedef ResponseCallContext<RangeResponse> RangeCallContext;

  RangeRequest get_request;
  get_request.set_key(key);
  if (with_prefix) {
    std::string range_end(key);
    int ascii = (int)range_end[range_end.length()-1];
    range_end.back() = ascii+1;
    get_request.set_range_end(range_end);
    get_request.set_sort_target(RangeRequest::SortTarget::RangeRequest_SortTarget_KEY);
    get_request.set_sort_order(RangeRequest::SortOrder::RangeRequest_SortOrder_ASCEND);
  }

  RangeCallContext call_ctx;
  etcd_ctx_await_pre(call_ctx);
  auto reader = kv_stub_->AsyncRange(&call_ctx.context, get_request, &c_queue_);
  reader->Finish(&call_ctx.response, &call_ctx.status, &call_ctx);
  etcd_ctx_await_end(call_ctx);

  if (!call_ctx.IsStatusOK()) {
    return KeyValues();
  }

  KeyValues kvs;
  for (auto& kv : call_ctx.response.kvs()) {
    kvs.emplace_back(kv);
  }
  return kvs;
}

int64_t EtcdClientV3::LeaseGrant(int ttl) {
  CHECK(co_can_yield);
  typedef ResponseCallContext<LeaseGrantResponse> LeaseGrantContext;

  etcdserverpb::LeaseGrantRequest request;
  request.set_ttl(ttl);

  LeaseGrantContext call_context;

  etcd_ctx_await_pre(call_context);
  auto reader = lease_stub_->AsyncLeaseGrant(&call_context.context, request, &c_queue_);
  reader->Finish(&(call_context.response), &(call_context.status), &call_context);
  etcd_ctx_await_end(call_context);

  return call_context.response.id();
}

RefKeepAliveContext EtcdClientV3::LeaseKeepalive(int64_t lease, int64_t interval) {
  CHECK(co_can_yield);

  RefKeepAliveContext call_ctx(new KeepAliveContext);

  if (!call_ctx->Initilize(lease_stub_.get(), &c_queue_)) {
    return nullptr;
  }
  VLOG(GLOG_VINFO) << __func__ << " keeaplive stream connted, id:" << lease;
  co_go std::bind(&KeepAliveContext::KeepAliveInternal, call_ctx, lease, interval);
  return call_ctx;
}

void EtcdClientV3::Finalize() {
  c_queue_.Shutdown();
}

void EtcdClientV3::Initilize(const Options& opt) {
  auto channel = grpc::CreateChannel(opt.addr, grpc::InsecureChannelCredentials());
  CHECK(channel);

  kv_stub_= KV::NewStub(channel);
  CHECK(kv_stub_);
  watch_stub_ = Watch::NewStub(channel);
  CHECK(watch_stub_);
  lease_stub_ = Lease::NewStub(channel);
  CHECK(lease_stub_);
  if (!opt.poll_in_loop) {
    thread_.reset(new std::thread(std::bind(&EtcdClientV3::PollCompleteQueueMain, this)));
  }
}

void EtcdClientV3::Sched() {
  bool ok = false;
  void* got_tag = nullptr;
  bool poll_continue = true;

  //VLOG(GLOG_VTRACE) << __func__ << " etcd grpc CompletionQueue poll start";
  do {
    auto deadline = std::chrono::system_clock::now() + std::chrono::microseconds(0);

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
      case grpc::CompletionQueue::TIMEOUT:
      case grpc::CompletionQueue::SHUTDOWN: {
        poll_continue = false;
        LOG(INFO) << __func__ << " etcd grpc CompletionQueue poll shutdown or timeout";
      } break;
      defaut:
        poll_continue = false;
        break;
    }
  } while(poll_continue);
  //VLOG(GLOG_VTRACE) << __func__ << " etcd grpc CompletionQueue poll end";
}

void EtcdClientV3::PollCompleteQueueMain() {
  void* got_tag;
  bool ok = false;

  // Block until the next result is available in the completion queue "cq".
  while (c_queue_.Next(&got_tag, &ok)) {
    // Verify that the request was completed successfully. Note that "ok"
    // corresponds solely to the request for updates introduced by Finish().
    LOG_IF(ERROR, !ok) << " grpc operation failed for context:" << got_tag;

    // The tag in this example is the memory location of the call object
    CallContext* call_context = static_cast<CallContext*>(got_tag);
    GPR_ASSERT(call_context);

    call_context->ResumeContext(ok);
  }
  LOG(INFO) << __func__ << " etcd grpc CompletionQueue poll end";
}


}//end lt
