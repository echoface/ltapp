#include "grpc_server_impl.h"
#include "grpcpp/security/server_credentials.h"
#include "proto/hello.grpc.pb.h"
#include <cstddef>
#include <algorithm>

namespace fw {

class RpcSayHelloCallContext {
public:
  RpcSayHelloCallContext()
    : responder_(&context_) {
  }

  bool BindRequest2Queue(Greeter::AsyncService* service, ServerCompletionQueue* cq) {
    service->RequestSayHello(&context_, &request_, &responder_, cq, cq, this);
    return true;
  }

private:
  ServerContext context_;
  HelloRequest request_;
  ServerAsyncResponseWriter<HelloReply> responder_;
};

RpcServerImpl::RpcServerImpl() {
}

void RpcServerImpl::WithCompleteQueueLoop(const std::vector<base::MessageLoop*>& loops) {
  queue_loops_ = loops;
}

void RpcServerImpl::Serve(const std::string& addr) {

  ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());

  builder.RegisterService(&service_);

  size_t pull_count = std::min(size_t(1), queue_loops_.size());
  for (size_t i = 0; i < pull_count; i++) {
    srv_cqs_.emplace_back(builder.AddCompletionQueue());
  }

  for (auto& complete_queue : srv_cqs_) {
    for (size_t i = 0; i < 1000; i++) { //add batch conetext wait fill by completequeue
      auto call_ctx = new RpcSayHelloCallContext();
      call_ctx->BindRequest2Queue(&service_, complete_queue.get());
    }
  }
  //cq_->AsyncNext(nullptr, &ok, nullptr);
  server_ = builder.BuildAndStart();

  for (auto loop : queue_loops_) {

  }
}


}
