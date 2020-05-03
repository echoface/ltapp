#ifndef LT_APPLICTION_CONTEXT_H_H
#define LT_APPLICTION_CONTEXT_H_H
#include <string>

class AppContext {
public:
  virtual ~AppContext() {};

  virtual std::string Name() const {return "main";};

  // when all prepare task finish successed
  virtual void OnStart() {};

  virtual void OnStop() {};
};

#endif
