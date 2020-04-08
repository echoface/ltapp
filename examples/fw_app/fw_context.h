#ifndef _LT_FW_APPLICTION_CONTEXT_H_H
#define _LT_FW_APPLICTION_CONTEXT_H_H

#include <glog/logging.h>

#include "app/app_context.h"
#include "app/config_loader.h"

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
