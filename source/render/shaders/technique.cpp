#include "mcv_platform.h"
#include "technique.h"
#include "render/textures/texture.h"

extern NamedValues<int> texture_slot_names;

// --------------------------------------------------------
class CTechniqueResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "tech"; }
  const char* getName() const override {
    return "Techniques";
  }
  IResource* create(const std::string& name) const override {
    CTechnique* new_res = new CTechnique();
    if (!new_res->create(name))
      return nullptr;
    new_res->setNameAndType(name, this);
    return new_res;
  }
};

template<>
const CResourceType* getResourceTypeFor<CTechnique>() {
  static CTechniqueResourceType resource_type;
  return &resource_type;
}

const CTechnique* CTechnique::current_active_tech = nullptr;

// --------------------------------------------------------
void CTechnique::activate() const {
  if (this == current_active_tech)
    return;
  vs.activate();

  // We might not have a valid ps, for example, when rendering the
  // shadows maps
  if (ps.isValid())
    ps.activate();
  else
    Render.ctx->PSSetShader(nullptr, nullptr, 0);

  activateRSConfig(rs_config);
  activateBlendConfig(blend_config);
  activateZConfig(z_config);
  current_active_tech = this;

  // Activate the predefined set of textures for this technique
  for (auto& ts : textures)
    ts.texture->activate(ts.slot);
}

void CTechnique::destroy() {
  vs.destroy();
  ps.destroy();
}

bool CTechnique::loadVS(const json& j) {
  std::string vs_fx = j.value("vs_fx", "data/shaders/objs.fx");
  std::string vs_fn = j.value("vs", "VS");
  std::string vdecl_name = j.value("vdecl", "PosNUv");
  if (!vs.create(vs_fx, vs_fn, vdecl_name))
    return false;
  return true;
}

bool CTechnique::loadPS(const json& j) {
  std::string vs_fx = j.value("vs_fx", "data/shaders/objs.fx");
  std::string ps_fx = j.value("ps_fx", vs_fx);
  std::string ps_fn = j.value("ps", "PS");

  // To support shadow map generation which does not use any PS
  if (ps_fn.empty())
    return true;

  if (!ps.create(ps_fx, ps_fn))
    return false;
  return true;
}

bool CTechnique::create(const std::string& filename) {
  TFileContext fc(filename);
  json new_jdef = loadJson(filename);
  return create(new_jdef);
}

bool CTechnique::create(const json& new_jdef) {

  jdef = new_jdef;

  if (!loadVS(jdef))
    return false;

  if (!loadPS(jdef))
    return false;

  rs_config = RSConfigFromString(jdef.value("rs", "default"));
  blend_config = BlendConfigFromString(jdef.value("blend", "default"));
  z_config = ZConfigFromString(jdef.value("z", "default"));

  uses_skin = vs.getVertexDecl()->name.find("Skin") != std::string::npos;

  for (int i = 0; i < texture_slot_names.size(); ++i) {
    auto item = texture_slot_names.nth(i);
    const char* label = item->name;
    if (jdef.count(label)) {
      std::string src = jdef[label];
      TTextureSlot ts;
      ts.slot = item->value;
      ts.texture = Resources.get(src)->as<CTexture>();
      textures.push_back(ts);
    };
  }

  /*if (jdef.count("lut")) {
    TTextureSlot ts;
    ts.texture = Resources.get(jdef["lut"])->as<CTexture>();
    ts.slot = TS_LUT;
    textures.push_back(ts);
  }*/

  return true;
}

void CTechnique::onFileChanged(const std::string& filename) {

  if (filename == this->getName()) {
    create(filename);
  }
  else {
    std::string vs_fx = jdef.value("vs_fx", "data/shaders/objs.fx");
    if (vs_fx == filename || filename == "data/shaders/common.fx")
      loadVS(jdef);

    std::string ps_fx = jdef.value("ps_fx", vs_fx );
    if (ps_fx == filename || filename == "data/shaders/common.fx")
      loadPS(jdef);

  }
}

void CTechnique::renderInMenu() {
  ::renderInMenu(rs_config);
  ::renderInMenu(blend_config);
  ::renderInMenu(z_config);
  ImGui::Text(jdef.dump(2, ' ').c_str());
  if (uses_skin)
    ImGui::Text("Uses Skin");
}

