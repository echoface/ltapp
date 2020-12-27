#include "keepalive_ctx.h"

#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"

#include "base/coroutine/wait_group.h"

namespace lt {

bool KeepAliveContext::Initilize(Lease::Stub* stub,
                                 grpc::CompletionQueue* c_queue) {
  CHECK(CO_CANYIELD);

  etcd_ctx_await_pre(*this);
  auto client_ctx = ClientContext();
  stream_ = stub->AsyncLeaseKeepAlive(client_ctx, c_queue, this);
  etcd_ctx_await_end(*this);
  return IsStatusOK();
}

void KeepAliveContext::KeepAliveInternal(int64_t lease_id,
                                         int64_t interval) {
  CHECK(CO_CANYIELD);

  auto wc = base::WaitGroup::New();
  wc->Add(2);

  //write keepalive request
  CO_GO base::MessageLoop::Current() << [&, wc]() {

    CallContext write_context;

    KeepaliveReq request;
    request.set_id(lease_id);

    int err_counter = 0;
    do {
      VLOG(GLOG_VTRACE) << "going to send keepalive request, lease:" << lease_id;

      etcd_ctx_await_pre(write_context);
      stream_->Write(request, &write_context);
      etcd_ctx_await_end(write_context);
      if (write_context.Success()) {
        err_counter = 0;
        LOG_EVERY_N(INFO, 100) << "send keepalive request, lease:" << lease_id;
      } else if (++err_counter >= 6) {
        LOG(ERROR) << "send keepalive request fail, lease:" << lease_id;
        break;
      }
      CO_SLEEP(interval);
    } while(!IsCanceled() && err_counter < 6);

    LOG(INFO) << "close keepalive writer, lease:" << lease_id;
    etcd_ctx_await_pre(write_context);
    stream_->WritesDone(&write_context);
    etcd_ctx_await_end(write_context);
    LOG(INFO) << "close keepalive writer done, lease:" << lease_id;

    cancel_ = true;
    wc->Done();
  };

  CO_GO base::MessageLoop::Current() << [&, wc]() {

    CallContext read_context;
    do {
      VLOG(GLOG_VTRACE) << "going to read keepalive response, lease:" << lease_id;
      etcd_ctx_await_pre(read_context);

      stream_->Read(MutableResponse(), &read_context);

      etcd_ctx_await_end(read_context);

      if (!Success()) {
        LOG(ERROR) << "read keepalive response failed, id:" << lease_id;
        break;
      }

      LOG_EVERY_N(INFO, 100) << "read keepalive response, lease:" << lease_id;
    } while(!IsCanceled());

    cancel_ = true;
    wc->Done();
  };

  wc->Wait();

  //got final status from server
  VLOG(GLOG_VTRACE) << __func__ << " going to close stream, id:" << lease_id;
  etcd_ctx_await_pre(*this);
  stream_->Finish(MutableStatus(), this);
  etcd_ctx_await_end(*this);

  VLOG(GLOG_VTRACE) << __func__ << " stream closed with status:" << DumpStatusMessage();
}

}//end lt
