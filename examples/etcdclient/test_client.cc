#include <gflags/gflags_declare.h>
#include <glog/logging.h>

#include <content/etcdv3/etcd_client.h>
#include <content/etcdv3/etcd_watcher.h>

#include <base/utils/string/str_utils.h>
#include <base/coroutine/coroutine_runner.h>
#include <unistd.h>

base::MessageLoop* loop = NULL;

void TestWatch(lt::EtcdClientV3* client) {

  lt::EtcdWatcher watcher(client);
  LOG(INFO) << ">>>>>>>>>> start watch /server/nodes/ changes";
  LOG(INFO) << "every 5 second will have a change event invoke";


  CO_GO loop << [&]() {
    lt::RefWatchContext ctx = watcher.Watch("/server/nodes/");

    WatchResponse response;
    do {
      bool ok = ctx->WaitEvent(&response);
      if (!ok) {
        LOG(ERROR) << "failed got response, id:" << ctx->WatchId();
        break;
      }
      LOG_EVERY_N(INFO, 10) << "watcher still alive" << response.events().size();
      for (const auto event : response.events()) {
        switch(event.type()) {
          case mvccpb::Event_EventType::Event_EventType_PUT:
            LOG_EVERY_N(INFO, 1000) << "Put, key:" << event.kv().key() << " value:" << event.kv().value();
            break;
          case mvccpb::Event_EventType::Event_EventType_DELETE:
            LOG_EVERY_N(INFO, 1000) << "Del, key:" << event.kv().key() << " value:" << event.kv().value();
            break;
          default:
            break;
        }
      }
    }while(1);
    LOG(INFO) << "<<<<<<<<<<<< end watch /server/nodes/ changes";
  };

#if 1

  CO_GO loop << [&]() {

    int64_t lease_id = client->LeaseGrant(120); //second
    LOG(INFO) << "lease grant id:" << lease_id;
    auto keepalive_ctx = client->LeaseKeepalive(lease_id, 5000); //ms

    LOG(INFO) << ">>>>>>>>>>>>>> put same kings to /server/nodes/";
    for (int cnt = 0; ; cnt++) {

      KeyValue kv;
      kv.set_key(base::StrUtil::Concat("/server/nodes/", cnt));
      kv.set_value(base::StrUtil::Concat("node_", cnt));
      kv.set_lease(lease_id);
      CO_SLEEP(5000);
      if (-1 == client->Put(kv)) {
        LOG(ERROR) << " put fialed:" << kv.value();
      }
      LOG_EVERY_N(INFO, 10) << "writer still alive";
    }

    LOG(INFO) << ">>>>>>>>>>>>>> list all /server key values";
    auto kvs = client->Range("/server", true);
    for (const auto& kv : kvs ) {
      LOG(INFO) << "key:" << kv.key() << " value:" << kv.value();
    }
    LOG(INFO) << "<<<<<<<<<<<<<< list all /server key values end";
    //loop.QuitLoop();
  };
#endif
  loop->WaitLoopEnd();
}

void TestKeepAlive(lt::EtcdClientV3* client) {
  lt::RefKeepAliveContext keepalive_ctx;
  CO_GO loop << [&]() {

    LOG(INFO) << "grant a lease ttl:" << 60 << "(s)";
    int64_t lease_id = client->LeaseGrant(60);
    LOG(INFO) << "lease grant id :" << lease_id;

    KeyValue kv;
    kv.set_key("/server/keepalive");
    kv.set_value(base::StrUtil::Concat("lease_", lease_id));
    kv.set_lease(lease_id);
    client->Put(kv);

    keepalive_ctx = client->LeaseKeepalive(lease_id, 5000);
    CHECK(keepalive_ctx);
    LOG(INFO) << "lease keepalive return:" << 0;
  };
  loop->WaitLoopEnd();
  LOG(INFO) << __func__ << " leave";
}

DECLARE_int32(v);
DECLARE_string(ep);

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " [watch|keepalive]" << std::endl;
    return -1;
  }

  FLAGS_v = 0;

  loop = new base::MessageLoop();
  loop->SetLoopName("grpc_poll");
  loop->Start();

  std::shared_ptr<lt::EtcdClientV3>  client(new lt::EtcdClientV3(loop));
  lt::EtcdClientV3::Options opt;
  opt.addr = "localhost:2379";
  opt.poll_in_loop = true;

  client->Initilize(opt);

  std::string case_name(argv[1]);
  if (case_name == "watch") {
    TestWatch(client.get());
  } else if (case_name == "keepalive") {
    TestKeepAlive(client.get());
  } else {
    std::cout << "test name not suported" << std::endl;
    return -1;
  }
  loop->WaitLoopEnd();
  delete loop;
  return 0;
}
