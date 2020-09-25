#include <memory>
#include <functional>
#include "lt_config.h"
#include <glog/logging.h>

namespace lt {


JsonConfSource::JsonConfSource(const std::string& file) {

}

JsonConfSource::~JsonConfSource() {
}

XmlConfig::XmlConfig(const std::string& file)
  : xml_file_(file) {
}

tinyxml2::XMLDocument* XmlConfig::GetDocument() const {
  return config_doc_.get();
}

XMLElement* XmlConfig::XMLRoot() const {
  return config_doc_ ? config_doc_->RootElement() : NULL;
}

bool XmlConfig::Load() {
  config_doc_.reset(new tinyxml2::XMLDocument);
  auto err = config_doc_->LoadFile(xml_file_.data());
  if (err != tinyxml2::XMLError::XML_SUCCESS) {
    LOG(ERROR) << " xml error:" << tinyxml2::XMLDocument::ErrorIDToName(err);
    return false;
  }
  LOG(INFO) << " xml config loaed";
  return true;
}

}
