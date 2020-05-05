#ifndef _LT_FW_APPLICTION_CONTEXT_H_H
#define _LT_FW_APPLICTION_CONTEXT_H_H

#include <glog/logging.h>
#include <app/etcdv3/proto/kv.pb.h>
#include <app/etcdv3/proto/rpc.grpc.pb.h>

#include "app/shell/app_context.h"
#include "app/config/lt_config.h"
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


  std::unique_ptr<KV::Stub> kv_stub_;
  std::unique_ptr<Watch::Stub> watch_stub_;
  std::unique_ptr<Lease::Stub> lease_stub_;
    //fw::ConfigManager rc_manager_;
  base::MessageLoop loop_;
};

}
#endif
