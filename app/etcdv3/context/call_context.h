#ifndef _LTAPP_ETCD_V3_CALL_CONTEXT_H_H
#define _LTAPP_ETCD_V3_CALL_CONTEXT_H_H

#include "google/protobuf/message.h"
#include "grpcpp/impl/codegen/async_stream.h"
#include "grpcpp/impl/codegen/async_unary_call.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"
#include "grpcpp/impl/codegen/config_protobuf.h"
#include "grpcpp/impl/codegen/status.h"
#include "base/message_loop/message_loop.h"
#include "grpcpp/grpcpp.h"
#include <atomic>
#include <bits/stdint-intn.h>
#include "etcdv3/proto/rpc.pb.h"
#include "etcdv3/proto/rpc.grpc.pb.h"

#include <functional>
#include <memory>
#include <sys/types.h>
#include <thread>
#include "base/coroutine/coroutine_runner.h"

using etcdserverpb::KV;
using etcdserverpb::Watch;
using etcdserverpb::Lease;

using etcdserverpb::WatchRequest;
using etcdserverpb::WatchResponse;

using grpc::protobuf::Message;
using grpc::ClientAsyncReaderWriter;
using grpc::CompletionQueue;

using mvccpb::Event;
using mvccpb::KeyValue;

namespace lt {

typedef std::function<void()> ContextResumer;
class CallContext {
  public:
    CallContext() : cancel_(false) {
    }
    virtual ~CallContext(){};

    void LockContext();
    void ResumeContext(bool ok);
    bool Success() const {return success_;}
  protected:
    bool success_ = false;
    std::atomic<bool> cancel_;
  private:
    ContextResumer resumer_;
};

template<class R>
class ResponseCallContext : public CallContext {
public:
  std::string DumpStatusMessage() {
    return status.error_message();
  }
  bool IsStatusOK() const {return status.ok();};
  const R& GetResponse() const {return response;}
  grpc::ClientContext* ClientContext() {return &context;}
public:
  R response;
  grpc::Status status;
  grpc::ClientContext context;
};


} //end namespace lt

#define etcd_ctx_await_pre(ctx) do {(ctx).LockContext();}while(0)

#define etcd_ctx_await_end(ctx)                                                                 \
  do {                                                                                          \
    VLOG(GLOG_VINFO) << "context:" << &(ctx) << " wait@" << __func__ << " line:" << __LINE__;   \
    co_pause;                                                                                   \
  }while(0)

#endif
