#ifndef _LTAPP_ETCD_V3_WATCH_CONTEXT_H_H
#define _LTAPP_ETCD_V3_WATCH_CONTEXT_H_H

#include "call_context.h"

namespace lt {

typedef ClientAsyncReaderWriter<WatchRequest,WatchResponse> WatcherStream;
typedef std::function<void (const mvccpb::Event&)> WatchEventFunc;

class EtcdWatcher;

class WatchContext : public CallContext {
public:
  WatchContext(Watch::Stub* stub,  base::MessageLoop* loop);

  //NOTE: must call in coro context, be clear what you a doing
  // return true and change event response will be fill
  bool WaitEvent(WatchResponse* response);

  // this all is safe for calling in anywhere
  // it will return at once, when change coming,
  // event_handler will be called
  void WaitEvent(WatchEventFunc event_handler);

  int64_t WatchId() const {return watch_id_;}

  void Cancel();
private:
  friend class EtcdWatcher;
  void wait_event_internal(WatchEventFunc handler);

  bool InitWithQueue(const WatchRequest& request,
                     grpc::CompletionQueue* c_queue);

  int64_t watch_id_ = 0;
  WatchResponse response_;
  grpc::ClientContext context_;
  Watch::Stub* stub_ = nullptr;
  base::MessageLoop* loop_ = nullptr;
  std::unique_ptr<WatcherStream> stream_;
};
typedef std::shared_ptr<WatchContext> RefWatchContext;

} //end lt

#endif
