#include "config_manager.h"
#include <pthread.h>
#include <unistd.h>
#include <base/message_loop/message_loop.h>
#include <utility>

namespace lt {

ConfigManager::ConfigManager(MessageLoop* loop,
                             ConfigSourceNameList& names)
  : loop_(loop) {
  for (const std::string& name : names) {
    config_sources_.insert(std::make_pair(name, 10));
  }
}

bool ConfigManager::RegisterWatcher(const std::string& source,
                                    ConfigWatcher* watcher) {

  if (loop_->IsInLoopThread()) {
    add_watcher(source, watcher);
    return true;
  }
  loop_->PostTask(FROM_HERE, [=]() {
    add_watcher(source, watcher);
  });
  return true;
}

ConfigSource ConfigManager::GetConfigSource(const std::string& source) const {
  if (has_source(source)) {
    return ConfigSource();
  }
  return ConfigSource();
}

bool ConfigManager::has_source(const std::string& source) const {
  return config_sources_.count(source) > 0;
}

void ConfigManager::add_watcher(const std::string& source,
                                ConfigWatcher* watcher) {
  watchers_[source].insert(watcher);
  //auto iter = config_sources_.find(source);
  //watcher->OnConfigChanged();
}

}
