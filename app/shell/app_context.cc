#include "app_context.h"
#include <type_traits>
#include <base/message_loop/message_loop.h>

void AppContext::MainLoopStarted(MessageLoop* loop) {
  main_ = loop;
}
