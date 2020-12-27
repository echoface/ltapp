#ifndef _LT_FW_APPLICTION_CONTEXT_H_H
#define _LT_FW_APPLICTION_CONTEXT_H_H

#include <glog/logging.h>

#include "content/app_context.h"
#include "content/config_loader.h"

class FWContext : public AppContext {
  public:
    FWContext() {};
    ~FWContext() {};


    void OnXmlConfigLoad(lt::XMLDocPtr doc) {
      LOG(INFO) << "XML document loaded";
    };
  private:
};

#endif
