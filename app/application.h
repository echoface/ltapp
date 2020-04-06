#ifndef LT_APPLICATION_H_H_
#define LT_APPLICATION_H_H_

#include <cstdint>
#include <functional>
#include <list>
#include <sstream>
#include <string>
#include "app_context.h"

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


class LtApplication {
public:
  LtApplication(AppContext* context);
  virtual ~LtApplication();

  LtApplication& With(const BootupItem& item);

private:
  AppContext* context_;
  std::list<BootupItem> bootup_tasks_;
};

} // namespace lt
#endif
