#ifndef _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H
#define _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H

#include "call_context.h"
#include <atomic>

namespace lt {

using ::etcdserverpb::LeaseKeepAliveRequest;
using ::etcdserverpb::LeaseKeepAliveResponse;

typedef ::grpc::ClientAsyncReaderWriter<
  LeaseKeepAliveRequest,
  LeaseKeepAliveResponse> KeepAliverStream;
typedef std::unique_ptr<KeepAliverStream> KeepAliverStreamPtr;

class KeepAliveContext : public CallContext {
public:
  bool Initilize(Lease::Stub* stub,
                 grpc::CompletionQueue* c_queue);

  void Cancel();

  bool IsStatusOK() const {return status.ok();};
  grpc::ClientContext* ClientContext() {return &context;}
  const LeaseKeepAliveResponse& GetResponse() const {
    return response;
  }

  std::string DumpStatusMessage() const;
public:
  grpc::Status status;
  std::atomic<bool> cancel;
  KeepAliverStreamPtr stream;
  grpc::ClientContext context;
  LeaseKeepAliveResponse response;
};

typedef std::shared_ptr<KeepAliveContext> RefKeepAliveContext;

}
#endif
