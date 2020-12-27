#ifndef _LTAPP_ETCD_V3_WATCH_CONTEXT_H_H
#define _LTAPP_ETCD_V3_WATCH_CONTEXT_H_H

#include "call_context.h"

namespace lt {

using WatchEventFunc=std::function<void (const mvccpb::Event&)>;

class EtcdWatcher;

class WatchContext : public ResponseCallContext<WatchResponse> {
public:
  using WatcherStream = ClientAsyncReaderWriter<WatchRequest,WatchResponse>;

  // NOTE: need call in coro context,
  // be clear what you doing return true
  // when response successe filled
  bool WaitEvent(WatchResponse* response);

  // this all is safe for calling in anywhere
  // it will return at once, when change coming,
  // event_handler will be called
  void WaitEvent(WatchEventFunc event_handler);

  int64_t WatchId() const {return watch_id_;}
private:
  friend class EtcdWatcher;

  WatchContext(Watch::Stub* stub, base::MessageLoop* loop);

  void WaitInternal(WatchEventFunc handler);

  bool InitWithQueue(const WatchRequest& request,
                     grpc::CompletionQueue* c_queue);

  int64_t watch_id_ = 0;
  Watch::Stub* stub_ = nullptr;
  base::MessageLoop* loop_ = nullptr;
  std::unique_ptr<WatcherStream> stream_;
};
using RefWatchContext = std::shared_ptr<WatchContext>;

} //end lt

#endif
