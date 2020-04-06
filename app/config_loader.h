#ifndef _LT_APPLICATION_CONFIGLOADER_H_H
#define _LT_APPLICATION_CONFIGLOADER_H_H

#include <memory>
#include <functional>

namespace tinyxml2 {
  class XMLDocument;
}

namespace lt {

class BootupItem;
typedef std::unique_ptr<tinyxml2::XMLDocument> XMLDocPtr;
typedef std::function<void(XMLDocPtr)> XMLLoadCallback; 

BootupItem WithXMLConfig(const std::string& file,
                         XMLLoadCallback callback,
                         bool sync = true);

}
#endif
