#include "unordered_map"
#include "etcd_registry.h"

namespace lt {

EtcdRegistry::EtcdRegistry() {}

bool Available() {
  return true;
}

std::vector<IPEndPoint> Lookup(const String& prefix) {
  return {};
}

void Register(const String& prefix, IPEndPoint& ep) {
  ;
}

void Unregister(const String& prefix, IPEndPoint& ep) {

}

void Subscribe(const String& prefix, RefNotifyListener listener) {

}

void Unsubscribe(const String& prefix, RefNotifyListener listener) {

}

}
