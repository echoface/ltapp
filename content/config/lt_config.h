#ifndef _LT_APP_CONFIG_RC_SOURCE_H_
#define _LT_APP_CONFIG_RC_SOURCE_H_

#include <string>
#include <memory>
#include <cinttypes>
#include "thirdparty/tinyxml2/tinyxml2.h"

/*
 *
 * struct XXConfig : public XmlReciever {
 *
 * }
 *
 * class LtXMLConfigurator {
 *
 * };
 *
 * LtXMLConfigurator loader;
 * loader.Register(name, receive);
 * loader.Load();
 * */

namespace lt {

//just a app config source interface
class LtAppConfig{
  public:
    virtual ~LtAppConfig() {};

};

using tinyxml2::XMLElement;

typedef std::unique_ptr<tinyxml2::XMLDocument> XMLDocPtr;

class JsonConfSource : public LtAppConfig {
  public:
    JsonConfSource(const std::string& file);
    ~JsonConfSource();
  private:
};

class XmlConfig : public LtAppConfig {
  public:
    XmlConfig(const std::string& file);

    bool Load();
    XMLElement* XMLRoot() const;
    tinyxml2::XMLDocument* GetDocument() const;
  private:
    XMLDocPtr config_doc_;
    const std::string xml_file_;
};

}
#endif
