#ifndef _LT_APPLICATION_H_H_
#define _LT_APPLICATION_H_H_

#include <sstream>
#include <string>

namespace lt {

/**
 * class YourAppContext : public LtAppContext {
 *
 * }
 *
 * int main() {
 *  YourAppContext context;
 *  LtApp app(context);
 *
 *  app.WithXMLConfig([&](context, xmlroot) -> bool {
 *  }, true);
 *
 * }
 * */
typedef struct StartupItem {
  std::string name;
}StartupItem;

class LtApp {
public:
  LtApp();
  virtual ~LtApp();
protected:

private:
};

} // namespace lt
#endif
