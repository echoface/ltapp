
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

using etcdserverpb::KV;
using etcdserverpb::Watch;
using etcdserverpb::Lease;

using etcdserverpb::WatchRequest;
using etcdserverpb::WatchResponse;

using grpc::protobuf::Message;
using grpc::ClientAsyncReaderWriter;

using mvccpb::Event;
using mvccpb::KeyValue;

namespace lt {

typedef std::function<void()> ContextResumer;
class CallContext {
  public:
    virtual ~CallContext(){};

    void LockContext();
    void ResumeContext(bool ok);
    bool Success() const {return success_;}
  protected:
    bool success_ = false;
  private:
    ContextResumer resumer_;
};

template<class R>
class ResponseCallContext : public CallContext {
public:
  std::string DumpStatusMessage() {
    return status.error_message();
  }
  bool IsOK() const {return status.ok();};
  const R& GetResponse() const {return response;}
  grpc::ClientContext* ClientContext() {return &context;}
public:
  R response;
  grpc::Status status;
  grpc::ClientContext context;
};

typedef ClientAsyncReaderWriter<WatchRequest,
                                WatchResponse> WatcherStream;
typedef std::function<void (const mvccpb::Event&)> WatchEventFunc;
class EtcdWatcher;

class WatchContext : public CallContext {
public:
  WatchContext(Watch::Stub* stub,  base::MessageLoop* loop);

  //NOTE: must call in coro context, be clear what you a doing
  // return true and change event response will be fill
  bool WaitEvent(WatchResponse* response);

  // this all is safe for calling in anywhere
  // it will return at once, when change coming,
  // event_handler will be called
  void WaitEvent(WatchEventFunc event_handler);

  int64_t WatchId() const {return watch_id_;}
private:
  friend class EtcdWatcher;
  void wait_event_internal(WatchEventFunc handler);

  bool InitWithQueue(const WatchRequest& request,
                     grpc::CompletionQueue* c_queue);

  int64_t watch_id_ = 0;
  WatchResponse response_;
  grpc::ClientContext context_;
  Watch::Stub* stub_ = nullptr;
  base::MessageLoop* loop_ = nullptr;
  std::unique_ptr<WatcherStream> stream_;
};
typedef std::shared_ptr<WatchContext> RefWatchContext;

} //end namespace lt

#define etcd_ctx_await_pre(ctx) do {(ctx).LockContext();}while(0)

#define etcd_ctx_await_end(ctx)                                                          \
  do {                                                                                   \
    LOG(INFO) << "context:" << &(ctx) << " wait@" << __func__ << " line:" << __LINE__;   \
    co_pause;                                                                            \
  }while(0)

#endif
