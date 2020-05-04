#include <bits/stdint-intn.h>
#include <glog/logging.h>

#include <app/etcdv3/etcd_client.h>
#include <app/etcdv3/etcd_watcher.h>

#include <base/utils/string/str_utils.h>
#include <base/coroutine/coroutine_runner.h>

base::MessageLoop* loop = NULL;

void TestWatch(lt::EtcdClientV3* client) {

  lt::EtcdWatcher watcher(client);

  co_go loop << [&]() {

    LOG(INFO) << ">>>>>>>>>> start watch /server/nodes/ changes";
    lt::RefWatchContext ctx = watcher.Watch("/server/nodes/");
    WatchResponse response;
    do {
      bool ok = ctx->WaitEvent(&response);
      if (!ok) {
        LOG(ERROR) << "failed got response, id:" << ctx->WatchId();
        break;
      }
      for (const auto event : response.events()) {
        switch(event.type()) {
          case Event_EventType::Event_EventType_PUT:
            LOG(INFO) << "Put, key:" << event.kv().key() << " value:" << event.kv().value();
            break;
          case Event_EventType::Event_EventType_DELETE:
            LOG(INFO) << "Del, key:" << event.kv().key() << " value:" << event.kv().value();
            break;
          default:
            break;
        }
      }
    }while(1);
    LOG(INFO) << "<<<<<<<<<<<< end watch /server/nodes/ changes";
  };

  co_go loop << [&]() {

    int64_t lease_id = client->LeaseGrant(60*5);
    LOG(INFO) << "lease grant id:" << lease_id;

    LOG(INFO) << ">>>>>>>>>>>>>> put same kings to /server/nodes/";
    for (size_t s = 0; s < 10; s++) {
      KeyValue kv;
      kv.set_key(base::StrUtil::Concat("/server/nodes/", s));
      kv.set_value(base::StrUtil::Concat("node_", s));
      kv.set_lease(lease_id);

      co_sleep(5000);
      LOG(INFO) << "put revision:" << client->Put(kv);
    }
    LOG(INFO) << "<<<<<<<<<<<<< put same kings to /server/nodes/";

    LOG(INFO) << ">>>>>>>>>>>>>> list all /server key values";
    auto kvs = client->Range("/server", true);
    for (const auto& kv : kvs ) {
      LOG(INFO) << "key:" << kv.key() << " value:" << kv.value();
    }
    LOG(INFO) << "<<<<<<<<<<<<<< list all /server key values";
    //loop.QuitLoop();
  };
  loop->WaitLoopEnd();
}

void TestKeepAlive(lt::EtcdClientV3* client) {
  co_go loop << [&]() {

    LOG(INFO) << "grant a lease ttl:" << 60 << "(s)";
    int64_t lease_id = client->LeaseGrant(60);
    LOG(INFO) << "lease grant id :" << lease_id;

    KeyValue kv;
    kv.set_key("/server/keepalive");
    kv.set_value(base::StrUtil::Concat("lease_", lease_id));
    kv.set_lease(lease_id);
    client->Put(kv);

    bool success = client->LeaseKeepalive(lease_id, 5000);
    LOG(INFO) << "lease keepalive return:" << success;
  };
  loop->WaitLoopEnd();
}


int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " [watch|keepalive]" << std::endl;
    return -1;
  }
  loop = new base::MessageLoop();
  loop->Start();

  std::shared_ptr<lt::EtcdClientV3>  client(new lt::EtcdClientV3(loop));
  client->Initilize({
    "localhost:2379"
  });

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

