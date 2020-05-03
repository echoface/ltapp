#ifndef _LT_APP_CONFIG_MANAGER_H_
#define _LT_APP_CONFIG_MANAGER_H_
#include <atomic>
#include <functional>
#include <initializer_list>
#include <map>
#include <mutex>
#include <set>
#include "lt_config.h"

namespace base {
  class MessageLoop;
}

namespace lt {

/*configmanager 并不管理config具体内容, 而是提供config的管理,
 * 加载已经变动通知, 支持提前注册多个config source */
;

using base::MessageLoop;

class ConfigWatcher {
  public:
    ConfigWatcher(){}
    ~ConfigWatcher(){}

    virtual void OnConfigChanged() = 0;
    virtual MessageLoop* Loop() const {return nullptr;} 
};
typedef std::set<ConfigWatcher*> ConfigWatcherSet;
typedef std::initializer_list<std::string> ConfigSourceNameList;

class ConfigManager {
  public:
    ConfigManager(MessageLoop* loop, ConfigSourceNameList& names);
    virtual ~ConfigManager() {};

    
    ConfigSource GetConfigSource(const std::string&) const;
    bool RegisterWatcher(const std::string&, ConfigWatcher*);
  private:
    bool has_source(const std::string& source) const;
    void add_watcher(const std::string&, ConfigWatcher*);
    MessageLoop* loop_;
    std::map<std::string, ConfigWatcherSet> watchers_;
    std::map<std::string, int> config_sources_;
};

}
#endif
