#include "keepalive_ctx.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"

namespace lt {

std::string KeepAliveContext::DumpStatusMessage() const{
  return status.error_message();
}

bool KeepAliveContext::Initilize(Lease::Stub* stub,
                                 grpc::CompletionQueue* c_queue) {
  CHECK(co_can_yield);

  etcd_ctx_await_pre(*this);
  stream = stub->AsyncLeaseKeepAlive(&context, c_queue, this);
  etcd_ctx_await_end(*this);
  return IsStatusOK();
}

void KeepAliveContext::Cancel() {
  cancel_.store(true);
  context.TryCancel();
}


}//end lt
