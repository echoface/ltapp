#ifndef _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H
#define _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H

#include "call_context.h"
#include "grpc/grpc.h"

namespace lt {

using ::etcdserverpb::LeaseKeepAliveRequest;
using ::etcdserverpb::LeaseKeepAliveResponse;

typedef ::grpc::ClientAsyncReaderWriter<
  LeaseKeepAliveRequest,
  LeaseKeepAliveResponse> KeepAliverStream;
typedef std::unique_ptr<KeepAliverStream> KeepAliverStreamPtr;

class KeepAliveContext : public CallContext {
public:
  void Cancel();

  bool IsStatusOK() const {return status.ok();};
  std::string DumpStatusMessage() const;
private:
  friend class EtcdClientV3;
  bool Initilize(Lease::Stub* stub,
                 grpc::CompletionQueue* c_queue);

  void KeepAliveInternal(int64_t lease, int64_t interval);

  grpc::ClientContext* ClientContext() {return &context;}
  const LeaseKeepAliveResponse& GetResponse() const {
    return response;
  }

  bool cancel;
  grpc::Status status;
  KeepAliverStreamPtr stream;
  grpc::ClientContext context;
  LeaseKeepAliveResponse response;
};

typedef std::shared_ptr<KeepAliveContext> RefKeepAliveContext;

}
#endif
