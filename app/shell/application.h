#ifndef LT_APPLICATION_H_H_
#define LT_APPLICATION_H_H_

#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include "app_context.h"
#include <base/message_loop/message_loop.h>

#include <condition_variable>

namespace lt {

enum class ExcRunType {
  kSequenceType  = 0x00,
  kAsyncRunType  = 0x01,
  kAsyncCoroType = 0x01 << 1,
};

struct BootTask;

using namespace std;
typedef std::function<bool()> BoolTask;
typedef std::unique_ptr<BootTask> BootTaskPtr;

class BootTask {
public:
  static BootTaskPtr New(const string& name,
                         BoolTask boot_task,
                         ExcRunType run_type = ExcRunType::kSequenceType);
  BoolTask     task;
  std::string  name;
  ExcRunType exc_type;
};


class Application {
public:
  Application(AppContext* context);
  virtual ~Application();

  Application& Prepare(BootTaskPtr item);
  Application& Cleanup(BootTaskPtr item);

  void Run();
protected:
  bool RunBootupTask();
  bool RunCleanupTask();
private:
  AppContext* context_;
  base::MessageLoop main_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::list<BootTaskPtr> on_start_;
  std::list<BootTaskPtr> on_finish_;
};

} // namespace lt
#endif
