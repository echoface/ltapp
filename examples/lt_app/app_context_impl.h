#ifndef _LT_FW_APPLICTION_CONTEXT_H_H
#define _LT_FW_APPLICTION_CONTEXT_H_H

#include <glog/logging.h>
#include <etcdv3/proto/rpc.grpc.pb.h>

#include "content/shell/app_context.h"
#include "content/config/lt_config.h"
#include "net_io/server/http_server/http_server.h"
#include <base/message_loop/repeating_timer.h>

using etcdserverpb::KV;
using etcdserverpb::Watch;
using etcdserverpb::Lease;

namespace fw {
class AppContextImpl : public AppContext {
  public:
    AppContextImpl();
    ~AppContextImpl() {};

    void Initialzie();

    bool InitConfig();
    bool InitClient();
    bool InitCoroPrepare();
    bool InitAsyncPrepare();

    void OnStart() override;
    void OnStop() override;

  private:
    lt::XmlConfig config_;

    //std::unique_ptr<Registry>
};

}
#endif
