
#include "base/closure/closure_task.h"
#include "base/message_loop/message_loop.h"
#include <atomic>
#include <base/coroutine/coroutine_runner.h>
#include <cstdint>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

static const int Vector_Size = 4 * 1024 * 1024 ; // 4k

class RawSampleManager {

public:
  RawSampleManager()
    : request_count_(Vector_Size, 0)
  {

  }
  virtual ~RawSampleManager() {}

  bool AddSampleRequest(const std::string &key, int sample_interval) {
    int64_t uidInt = this->GetHashKey(key);
    int index = uidInt % Vector_Size;

    bool need_sample = false;
    int x = __sync_fetch_and_add(&request_count_[index], 1);
    //int x = request_count_[index].fetch_add(1);
    if (x % sample_interval == 0) {
      need_sample = true;
    }
    return need_sample;
  }

private:
  int64_t GetHashKey(const std::string &uid) {
    uint64_t result = 0;
    for (auto c : uid) {
      result += int(c);
    }
    return result;
  }
  //uint16_t request_count_[Vector_Size];
  std::vector<uint16_t> request_count_;
  //std::vector<std::atomic<uint16_t>> request_count_;
};

int main(int argc, char** argv) {
  std::vector<base::MessageLoop*> threads;
  for (int i = 0; i < 5; i++) {
    auto loop = new base::MessageLoop();
    loop->Start();
    threads.push_back(loop);
  }

  RawSampleManager*  smg = new RawSampleManager();

  auto func = [&]() {
    int64_t i = 10000000;
    while(i-- > 0) {
      smg->AddSampleRequest("abc", 1000);
      smg->AddSampleRequest("abc2", 1000);
      smg->AddSampleRequest("abc3", 1000);
      smg->AddSampleRequest("abc4", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
      sleep(0);
      smg->AddSampleRequest("abc", 1000);
      smg->AddSampleRequest("abc2", 1000);
      smg->AddSampleRequest("abc3", 1000);
      smg->AddSampleRequest("abc4", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
      smg->AddSampleRequest("abc5", 1000);
    }
  };

  for (auto loop : threads) {
    //loop->PostTask(NewClosure(func));
    co_go loop << func;
  }

  for (auto loop : threads) {
    loop->WaitLoopEnd();
  }
  return 0;
}

