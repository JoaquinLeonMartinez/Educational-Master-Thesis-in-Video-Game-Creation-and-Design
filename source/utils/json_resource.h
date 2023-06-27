#pragma once

class CJson : public IResource {
  json jdata;
public:
  CJson(const std::string& filename);
  void renderInMenu() override;
  void onFileChanged(const std::string& filename) override;
  const json& getJson() const { return jdata; }
};
