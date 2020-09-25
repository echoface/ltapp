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
};

void AppContextImpl::OnStop() {
}

}
