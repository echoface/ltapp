#ifndef _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H
#define _LTAPP_ETCD_V3_KEEPALIVE_CONTEXT_H_H

#include "call_context.h"
#include "grpc/grpc.h"

namespace lt {

using KeepaliveReq = ::etcdserverpb::LeaseKeepAliveRequest;
using KeepaliveRes = ::etcdserverpb::LeaseKeepAliveResponse;
using KeepaliveClientBidiReactor =
  grpc::ClientBidiReactor<KeepaliveReq, KeepaliveRes>;

class KeepAliveContext;
using RefKeepAliveContext = std::shared_ptr<KeepAliveContext>;

class KeepAliveContext
    : public CallContext,
      public KeepaliveClientBidiReactor,
      public EnableShared(KeepAliveContext) {
public:
  static std::shared_ptr<KeepAliveContext>
    New(base::MessageLoop* loop, Lease::Stub *stub);

  bool Start(int64_t lease, int64_t interval);

  __CO_WAIT__ void Wait();
protected:
  friend class EtcdClientV3;
  KeepAliveContext(base::MessageLoop* loop, Lease::Stub *stub);

  void keepalive(int64_t lease, int64_t interval);

  void OnReadDone(bool ok) override;

  void OnWriteDone(bool ok) override;

  void OnWritesDoneDone(bool /*ok*/) override;

  void OnDone(const grpc::Status &s) override;

private:

  int64_t lease_id_;
  int64_t interval_;
  Lease::Stub *stub_;

  KeepaliveReq request_;
  KeepaliveRes response_;
  grpc::ClientContext client_ctx_;

  base::MessageLoop* loop_ = nullptr;
};

} //end lt
#endif
