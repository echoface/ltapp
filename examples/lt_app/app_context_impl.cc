#include "app_context_impl.h"
#include "grpcpp/grpcpp.h"
#include <base/coroutine/coroutine_runner.h>


DEFINE_string(config, "manifest.xml", "config file path for tihs app");
DEFINE_string(client, "clients", "config file path for tihs app");

namespace fw {

AppContextImpl::AppContextImpl()
  : config_(FLAGS_config) {
}

void AppContextImpl::Initialzie() {

}

bool AppContextImpl::InitConfig() {
  LOG(INFO) << __func__ << " run";
  return config_.Load();
}

bool AppContextImpl::InitCoroPrepare() {
  LOG(INFO) << __func__ << " run";
  return true;
}

bool AppContextImpl::InitAsyncPrepare() {
  LOG(INFO) << __func__ << " run";
  return true;
}

bool AppContextImpl::InitClient() {
  LOG(INFO) << __func__ << " run";
  const tinyxml2::XMLElement* root = config_.XMLRoot();
  CHECK(root);

  auto clients = root->FirstChildElement(FLAGS_client.data());
  return clients != nullptr;
}

void AppContextImpl::OnStart() {

  auto channel = grpc::CreateChannel("localhost:2379", grpc::InsecureChannelCredentials());
  kv_stub_= KV::NewStub(channel);
  watch_stub_ = Watch::NewStub(channel);
  lease_stub_ = Lease::NewStub(channel);
  loop_.SetLoopName("app_context");

  co_go &loop_ << [&]() {

#if 0
    etcdv3::ActionParameters params;
    params.key.assign(key);
    params.value.assign(value);
    params.kv_stub = stub_.get();

    if(ttl > 0)
    {
      auto res = leasegrant(ttl).get();
      if(!res.is_ok())
      {
        return pplx::task<etcd::Response>([res]()
                                          {
                                            return etcd::Response(res.error_code(), res.error_message().c_str());
                                          });
      }
      else
      {
        params.lease_id = res.value().lease();
      }
    }
    std::shared_ptr<etcdv3::AsyncSetAction> call(new etcdv3::AsyncSetAction(params));
    return Response::create(call);
#endif

  };
};

void AppContextImpl::OnStop() {
}

}
