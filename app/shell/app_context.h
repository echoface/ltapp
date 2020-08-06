#ifndef LT_APPLICTION_CONTEXT_H_H
#define LT_APPLICTION_CONTEXT_H_H

#include <string>
namespace base {
  class MessageLoop;
}
using base::MessageLoop;
class AppContext {
public:
  virtual ~AppContext() {};

  virtual std::string Name() const {return "main";};

  /*shell 本身不拥有或创建任何资源, 但是为了实现各种初始化
   * 任务的运行处理, 需要基于MessageLoop/CoroutineCtx 完成
   * 初始化任务, 所以需要Shell具体的实现者提供此对象*/
  virtual void MainLoopStarted(MessageLoop*);

  // when all prepare task finish successed
  virtual void OnStart() {};

  virtual void OnStop() {};
protected:
  MessageLoop* main_;
};

#endif
