#include <gflags/gflags_declare.h>
#include <glog/logging.h>

#include <content/etcdv3/etcd_client.h>
#include <content/etcdv3/etcd_watcher.h>

#include <base/utils/string/str_utils.h>
#include <base/coroutine/coroutine_runner.h>
#include <unistd.h>

base::MessageLoop *loop = NULL;
lt::RefWatchContext watch_ctx = nullptr;

void PusKeyEvery(lt::EtcdClientV3 *client, int cnt, int sec) {
  int64_t lease_id = client->LeaseGrant(120); //second
  LOG(INFO) << "lease grant id:" << lease_id;
  auto keepalive_ctx = client->LeaseKeepalive(lease_id, 5000); //ms

  LOG(INFO) << ">>>>>>>>>>>>>> put same kings to /server/nodes/";
  while (cnt > 0) {
    cnt--;

    KeyValue kv;
    kv.set_key(base::StrUtil::Concat("/server/nodes/", cnt));
    kv.set_value(base::StrUtil::Concat("node_", cnt));
    kv.set_lease(lease_id);

    CO_SLEEP(sec * 1000);
    if (-1 == client->Put(kv)) {
      LOG(ERROR) << " put fialed:" << kv.value();
    }
    LOG_EVERY_N(INFO, 10) << __FUNCTION__ << " still work";
  }

  LOG(INFO) << ">>>>>>>>>>>>>> list all /server key values";
  auto kvs = client->Range("/server", true);
  for (const auto& kv : kvs ) {
    LOG(INFO) << "key:" << kv.key() << " value:" << kv.value();
  }
  LOG(INFO) << "<<<<<<<<<<<<<< list all /server key values end";

  CO_SLEEP(1);
  CHECK(watch_ctx);
  LOG(INFO) << ">>>>>>>>>>>>>> start stopwatch changes >>>>>>";
  CHECK(watch_ctx->StopWatch());
  LOG(INFO) << "<<<<<<<<<<<<<< stop watch changes done <<<<<<";
  watch_ctx.reset();

  KeyValue kv;
  kv.set_key(base::StrUtil::Concat("/server/nodes/", 99999));
  kv.set_value("should_not_be_watched_change_node");
  kv.set_lease(lease_id);
  LOG_IF(ERROR, -1 == client->Put(kv)) << "put last node changes fail";
};

void OnNodeChange(const mvccpb::Event &event) {
  switch(event.type()) {
    case mvccpb::Event_EventType::Event_EventType_PUT:
      LOG(INFO) << "Put, key:" << event.kv().key() << " value:" << event.kv().value();
      break;
    case mvccpb::Event_EventType::Event_EventType_DELETE:
      LOG(INFO) << "Del, key:" << event.kv().key() << " value:" << event.kv().value();
      break;
    default:
      break;
  }
}

void TestWatch(lt::EtcdClientV3* client) {
  lt::EtcdWatcher watcher(client);
  LOG(INFO) << ">>>>>>>>>> start watch /server/nodes/ changes";
  LOG(INFO) << "every 5 second will have a change event invoke";

  CO_GO std::bind(PusKeyEvery, client, 20, 1);
  watch_ctx = watcher.Watch("/server/nodes/", true, OnNodeChange);
  CHECK(watch_ctx);
  LOG(INFO) << "<<<<<<<<<<<< end watch /server/nodes/ changes";
}

void TestKeepAlive(lt::EtcdClientV3* client) {
  lt::RefKeepAliveContext keepalive_ctx;

  LOG(INFO) << "grant a lease ttl:" << 60 << "(s)";
  int64_t lease_id = client->LeaseGrant(60);
  LOG(INFO) << "grant lease id :" << lease_id;

  KeyValue kv;
  kv.set_key("/server/keepalive");
  kv.set_value(base::StrUtil::Concat("lease_", lease_id));
  kv.set_lease(lease_id);
  client->Put(kv);

  keepalive_ctx = client->LeaseKeepalive(lease_id, 2000);
  CHECK(keepalive_ctx);

  int n = 0;
  while (n++ < 30) {
    CO_SLEEP(1000);
  }
  keepalive_ctx->Cancel();

  keepalive_ctx->Wait();
  LOG(INFO) << __func__ << " leave, sucess:" << keepalive_ctx->Success();
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
  std::cout << "grpc poll loop started" << std::endl;

  std::shared_ptr<lt::EtcdClientV3>  client(new lt::EtcdClientV3(loop));
  lt::EtcdClientV3::Options opt;
  opt.addr = "localhost:2379";
  opt.poll_in_loop = true;

  client->Initilize(opt);

  std::string case_name(argv[1]);
  if (case_name == "watch") {
    CO_GO std::bind(TestWatch, client.get());
  } else if (case_name == "keepalive") {
    CO_GO std::bind(TestKeepAlive, client.get());
  } else {
    std::cout << "test name not suported" << std::endl;
    return -1;
  }
  loop->WaitLoopEnd();
  delete loop;
  return 0;
}
