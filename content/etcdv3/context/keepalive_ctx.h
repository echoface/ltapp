#ifndef _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H
#define _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H

#include "call_context.h"
#include "grpc/grpc.h"

namespace lt {

using KeepaliveReq = ::etcdserverpb::LeaseKeepAliveRequest;
using KeepaliveRes = ::etcdserverpb::LeaseKeepAliveResponse;

class KeepAliveContext : public ResponseCallContext<KeepaliveRes> {
private:
  friend class EtcdClientV3;

  using KeepAliverStream =
    grpc::ClientAsyncReaderWriter<KeepaliveReq, KeepaliveRes>;

  using KeepAliverStreamPtr = std::unique_ptr<KeepAliverStream>;

  __CO_WAIT__
  bool Initilize(Lease::Stub* stub,
                 grpc::CompletionQueue* c_queue);

  __CO_WAIT__
  void KeepAliveInternal(int64_t lease, int64_t interval);

private:
  KeepAliverStreamPtr stream_;
};
using RefKeepAliveContext = std::shared_ptr<KeepAliveContext>;

} //end lt
#endif
