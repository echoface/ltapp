#include "keepalive_ctx.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/impl/codegen/completion_queue.h"

#include "base/coroutine/wait_group.h"

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

void KeepAliveContext::KeepAliveInternal(int64_t lease_id,
                                         int64_t interval) {
  CHECK(co_can_yield);

  base::WaitGroup wc;
  wc.Add(2);

  //write keepalive request
  co_go [&]() {
    CallContext write_context;

    LeaseKeepAliveRequest request;
    request.set_id(lease_id);

    int err_counter = 0;
    do {
      VLOG(GLOG_VTRACE) << "going to send keepalive request, lease:" << lease_id;
      etcd_ctx_await_pre(write_context);
      stream->Write(request, &write_context);
      etcd_ctx_await_end(write_context);
      if (write_context.Success()) {
        err_counter = 0;
        LOG_EVERY_N(INFO, 100) << "send keepalive request, lease:" << lease_id;
      } else if (++err_counter >= 6) {
        LOG(ERROR) << "send keepalive request fail, lease:" << lease_id;
        break;
      }
      co_sleep(interval);
    } while(!cancel && err_counter < 6);

    LOG(INFO) << "close keepalive writer, lease:" << lease_id;
    etcd_ctx_await_pre(write_context);
    stream->WritesDone(&write_context);
    etcd_ctx_await_end(write_context);
    LOG(INFO) << "close keepalive writer done, lease:" << lease_id;

    cancel = true;
    wc.Done();
  };

  co_go [&]() {

    CallContext read_context;
    do {
      VLOG(GLOG_VTRACE) << "going to read keepalive response, lease:" << lease_id;
      etcd_ctx_await_pre(read_context);
      stream->Read(&response, &read_context);
      etcd_ctx_await_end(read_context);

      if (!Success()) {
        LOG(ERROR) << "read keepalive response failed, id:" << lease_id;
        break;
      }
      LOG_EVERY_N(INFO, 100) << "read keepalive response, lease:" << lease_id;
    } while(!cancel);

    cancel = true;
    wc.Done();
  };

  wc.Wait();

  //got final status from server
  VLOG(GLOG_VTRACE) << __func__ << " going to close stream, id:" << lease_id;
  etcd_ctx_await_pre(*this);
  stream->Finish(&status, this);
  etcd_ctx_await_end(*this);
  VLOG(GLOG_VTRACE) << __func__ << " stream closed with status:" << DumpStatusMessage();
}

void KeepAliveContext::Cancel() {
  cancel_ = true;
  context.TryCancel();
}


}//end lt
