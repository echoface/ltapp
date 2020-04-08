#ifndef LT_APPLICATION_H_H_
#define LT_APPLICATION_H_H_

#include <cstdint>
#include <functional>
#include <list>
#include <sstream>
#include <string>
#include "app_context.h"
#include <base/message_loop/message_loop.h>

namespace lt {

typedef std::function<bool()> BootupPipeline;

enum class ExcRunType {
  kSequenceType  = 0x00,
  kAsyncRunType  = 0x01,
  kAsyncCoroType = 0x01 << 1,
};

typedef struct BootupItem {
  std::string  name;
  ExcRunType exc_type;
  BootupPipeline task;
  std::int32_t status;
  std::string  message;
} BootupItem;


class Application {
public:
  Application(AppContext* context);
  virtual ~Application();

  Application& With(const BootupItem& item);

  void Run();
private:
  AppContext* context_;
  base::MessageLoop main_;
  std::list<BootupItem> bootup_tasks_;
};

} // namespace lt
#endif
