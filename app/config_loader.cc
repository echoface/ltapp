
#include "app_context.h"
#include "application.h"

#include "base/utils/string/str_utils.h"
#include "thirdparty/tinyxml2/tinyxml2.h"

#include <functional>
#include <type_traits>

#include "config_loader.h"

namespace lt {

BootupItem WithXMLConfig(const std::string& file,
                         XMLLoadCallback callback,
                         bool sync) {

  BootupPipeline task = [=]() -> bool {
    XMLDocPtr document(new tinyxml2::XMLDocument);
    auto err = document->LoadFile(file.data());
    if (err != tinyxml2::XMLError::XML_SUCCESS) {
      LOG(ERROR) << " load config error:"
        << tinyxml2::XMLDocument::ErrorIDToName(err);
      return false;
    }
    callback(std::move(document));
    return true;
  };

  auto exc_type =
    sync ? ExcRunType::kSequenceType : ExcRunType::kAsyncRunType;

  BootupItem item = {
    .name      = base::StrUtil::Concat("load config:", file),
    .exc_type  = exc_type,
    .task      = task,
    .status    = 0,
  };
  return item;
}


}
