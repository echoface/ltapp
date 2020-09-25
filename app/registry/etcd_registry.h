#ifndef _LT_APP_REGISTRY_ETCD_H_H
#define _LT_APP_REGISTRY_ETCD_H_H

#include <memory>
#include "registry.h"
#include <etcdv3/etcd_client.h>
#include <etcdv3/etcd_watcher.h>

namespace lt {

class EtcdRegistry final : public Registry{
public:
  EtcdRegistry();

  bool Available() override {
    return true;
  }

  std::vector<IPEndPoint> Lookup(const String& prefix) override;
  void Register(const String& prefix, IPEndPoint& ep) override;
  void Unregister(const String& prefix, IPEndPoint& ep) override;
  void Subscribe(const String& prefix, RefNotifyListener listener) override;
  void Unsubscribe(const String& prefix, RefNotifyListener listener) override;
private:
  std::unique_ptr<EtcdClientV3> client_;
};

}
#endif
