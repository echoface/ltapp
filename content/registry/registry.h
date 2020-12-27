#ifndef _LT_REGISTRY_H_
#define _LT_REGISTRY_H_

#include <memory>
#include <string>
#include <vector>
#include <net_io/base/ip_endpoint.h>

namespace lt {

using net::IPEndPoint;
typedef std::string String;

class NotifyListener {
public:
};

typedef std::string URL;
typedef std::shared_ptr<NotifyListener> RefNotifyListener;

class Registry {
public:
  Registry();
  virtual ~Registry();

  virtual bool Available() = 0;

  virtual std::vector<IPEndPoint> Lookup(const String& prefix) = 0;
  virtual void Register(const String& prefix, IPEndPoint& ep) = 0;
  virtual void Unregister(const String& prefix, IPEndPoint& ep) = 0;
  virtual void Subscribe(const String& prefix, RefNotifyListener listener) = 0;
  virtual void Unsubscribe(const String& prefix, RefNotifyListener listener) = 0;
};

} //end namespace lt
#endif
