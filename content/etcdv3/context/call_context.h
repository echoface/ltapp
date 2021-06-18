#ifndef _LTAPP_ETCD_V3_CALL_CONTEXT_H_H
#define _LTAPP_ETCD_V3_CALL_CONTEXT_H_H

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "base/coroutine/coroutine_runner.h"
#include "base/message_loop/message_loop.h"
#include "etcdv3/proto/rpc.grpc.pb.h"
#include "google/protobuf/message.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/impl/codegen/async_stream.h"
#include "grpcpp/impl/codegen/async_unary_call.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"
#include "grpcpp/impl/codegen/config_protobuf.h"
#include "grpcpp/impl/codegen/status.h"

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

#ifndef __CO_WAIT__
#define __CO_WAIT__
#endif

namespace lt {

class CallContext {
public:
  CallContext() : cancel_(false), done_(false) {}

  virtual ~CallContext(){};

  //return true when resume has been called
  bool LockContext();

  void ResumeContext(bool ok);

  void Cancel() { cancel_.store(true); }

  bool Canceled() const {return cancel_.load();}

  bool Success() const { return success_; }

  bool Done() const { return done_.load(); }

  void Reset();
protected:
  bool success_ = false;

  std::atomic<bool> cancel_;

private:
  //this is lite enough,
  //almost 100% aquire lock at first try
  base::SpinLock mu_;

  // mean resume run or not
  std::atomic<bool> done_;
  base::LtClosure resumer_;
};

template<class R>
class ResponseCallContext : public CallContext {
public:
  std::string DumpStatusMessage() {
    return status_.error_message();
  }

  bool IsStatusOK() const {return status_.ok();};

  grpc::Status* MutableStatus() {return &status_;}

  R* MutableResponse() {return &response_;}

  const R& GetResponse() const {return response_;}

  grpc::ClientContext* ClientContext() {return &context_;}

  void Cancel() { CallContext::Cancel(); context_.TryCancel();}
protected:
  R response_;

  grpc::Status status_;

  grpc::ClientContext context_;
};

} //end namespace lt

#endif
