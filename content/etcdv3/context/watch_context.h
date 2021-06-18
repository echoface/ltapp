#ifndef _LTAPP_ETCD_V3_WATCH_CONTEXT_H_H
#define _LTAPP_ETCD_V3_WATCH_CONTEXT_H_H

#include "call_context.h"

namespace lt {

using WatchEventFunc = std::function<void(const mvccpb::Event &)>;
using WatchStreamReactor = grpc::ClientBidiReactor<WatchRequest, WatchResponse>;

class EtcdWatcher;

class WatchContext
  : public WatchStreamReactor,
    public EnableShared(WatchContext) {
public:
  WatchContext(Watch::Stub *stub, base::MessageLoop *loop);

  //return fasle when watch failed
  __CO_WAIT__ bool Start(const WatchRequest& /*req*/,
                         const WatchEventFunc& /*callbcak*/);

  __CO_WAIT__ bool StopWatch();

  int64_t WatchId() const { return watch_id_; }

protected:
  __CO_WAIT__ void watch_change_loop();

  void OnWriteDone(bool ok) override;

  void OnWritesDoneDone(bool /*ok*/) override;

  void OnReadDone(bool ok) override;

  void OnDone(const grpc::Status &s) override;
private:
  WatchEventFunc handler_;

  CallContext read_ctx_;
  CallContext write_ctx_;

  grpc::ClientContext client_ctx_;

  int64_t watch_id_ = 0;
  Watch::Stub* stub_ = nullptr;
  base::MessageLoop *loop_ = nullptr;
};
using RefWatchContext = std::shared_ptr<WatchContext>;

} //end lt

#endif
