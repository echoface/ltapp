#ifndef _LT_APP_FW_GRPC_SERVER_IMPL_H
#define _LT_APP_FW_GRPC_SERVER_IMPL_H

#include "grpcpp/grpcpp.h"
#include "proto/hello.pb.h"
#include "proto/hello.grpc.pb.h"
#include <memory>
#include <vector>

namespace base {
  class MessageLoop;
}

namespace fw {

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

typedef std::unique_ptr<ServerCompletionQueue> GrpcCompleteQueuePtr;

class RpcServerImpl final {
public:
  RpcServerImpl();

  /*return at once when use loop dump completequeu,
   *otherwise will block here and wait for server stoped*/
  void Serve(const std::string& addr);

  /* 当配置了MessageLoop时, 会使用MessageLoop的事件循环来PumpGRPC消息,来Poll*/
  void WithCompleteQueueLoop(const std::vector<base::MessageLoop*>& loop);
private:
  std::unique_ptr<Server> server_;
  std::vector<GrpcCompleteQueuePtr> srv_cqs_;

  Greeter::AsyncService service_;

  std::vector<base::MessageLoop*> queue_loops_;
};

}
#endif
