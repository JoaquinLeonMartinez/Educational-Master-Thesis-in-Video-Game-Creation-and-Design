#include "mcv_platform.h"
#include "json_resource.h"

class CJsonResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "json"; }
  const char* getName() const override {
    return "JSONs";
  }
  IResource* create(const std::string& filename) const override {
    CJson* new_res = new CJson( filename );
    new_res->setNameAndType(filename, this);
    return new_res;
  }
};

CJson::CJson(const std::string& filename) {
  jdata = loadJson(filename);
}

void CJson::onFileChanged(const std::string& filename) {
  if (filename != getName())
    return;
  jdata = loadJson(filename);
}

void CJson::renderInMenu() {
  std::string jstr = jdata.dump(2);
  ImGui::Text("%s", jstr.c_str());
}

template<>
const CResourceType* getResourceTypeFor<CJson>() {
  static CJsonResourceType resource_type;
  return &resource_type;
}



