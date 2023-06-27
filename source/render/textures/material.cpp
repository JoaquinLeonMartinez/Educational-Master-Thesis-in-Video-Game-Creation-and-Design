#include "mcv_platform.h"
#include "material.h"
#include "texture.h"
#include "utils/json_resource.h"
#include "material_mixing.h"
#include "components/common/comp_buffers.h"
#include "render/compute/gpu_buffer.h"

class CMaterialResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "material"; }
  const char* getName() const override {
    return "Materials";
  }
  IResource* create(const std::string& name) const override {

    // This is the factory of all materials... This time will 
    // check the type attribute looking for custom types
    auto j = loadJson(name);

    std::string mat_type = j.value("type", "std");

    CMaterial* new_res = nullptr;
    if (mat_type == "std") {
      new_res = new CMaterial();
    }
    else if (mat_type == "mix") {
      new_res = new CMaterialMixing();
    }
    else {
      fatal("Invalid material type %s found at", mat_type.c_str(), name.c_str());
    }

    bool is_ok = new_res->create(j);
    if (!is_ok)
      return nullptr;

    new_res->setNameAndType(name, this);
    return new_res;
  }
};

template<>
const CResourceType* getResourceTypeFor<CMaterial>() {
  static CMaterialResourceType resource_type;
  return &resource_type;
}

// -------------------------------------------------------
using RenderCategoryNames = NamedValues<eRenderCategory>;
RenderCategoryNames::TEntry render_category_names[] = {
  {eRenderCategory::CATEGORY_SOLIDS, "solid"},
  {eRenderCategory::CATEGORY_UI, "ui"},
  {eRenderCategory::CATEGORY_DISTORSIONS, "distorsions"},
  {eRenderCategory::CATEGORY_DECALS, "decals"},
  {eRenderCategory::CATEGORY_TRANSPARENTS, "transparent"},
  {eRenderCategory::CATEGORY_SHADOWS, "shadows"},
  {eRenderCategory::CATEGORY_SHINE, "shine"},
};
RenderCategoryNames category_names(render_category_names, sizeof(render_category_names) / sizeof(RenderCategoryNames::TEntry));

// -------------------------------------------------------
// I'm assuming the normal = albedo + 1, metallic = normal + 1, roughness = metallic + 1
using TextureSlotNames = NamedValues<int>;
TextureSlotNames::TEntry texture_slot_names_data[] = {
  {TS_ALBEDO, "albedo"},
  {TS_NORMAL, "normal"},
  {TS_METALLIC, "metallic"},
  {TS_ROUGHNESS, "roughness"},
  {TS_EMISSIVE, "emissive"},
  {TS_ENVIRONMENT_MAP, "envmap"},
  {TS_IRRADIANCE_MAP, "irradiance"},
  {TS_NOISE_MAP, "noise"},
};
TextureSlotNames texture_slot_names(texture_slot_names_data, sizeof(texture_slot_names_data) / sizeof(TextureSlotNames::TEntry));

// ----------------------------------------------------------
const char* CMaterial::getCategoryName() const {
  return category_names.nameOf(category);
}

void CMaterial::destroy() {
  if (ctes_material) {
    ctes_material->destroy();
    ctes_material = nullptr;
  }
}

// ----------------------------------------------------------
bool CMaterial::create(const json& j) {

  assert(j.count("textures") || fatal("material %s requires attribute textures\n", name.c_str()));

  // Set all textures to a null value
  for (int i = 0; i < max_textures; ++i) {
    textures[i] = nullptr;
    srvs[i] = nullptr;
  }

  if (j.count("textures")) {
    const json& jtextures = j["textures"];
    for (auto it : jtextures.items() ) {
      std::string slot = it.key();
      std::string texture_name = it.value();

      int ts = texture_slot_names.valueOf(slot.c_str());

      assert((ts >= 0 && ts < max_textures) || fatal("Material %s has an invalid texture slot %s\n", name.c_str(), slot.c_str()));
      textures[ts] = Resources.get(texture_name)->as<CTexture>();
    }
  }

  // Fill gaps
  if (!textures[TS_ALBEDO])
    textures[TS_ALBEDO] = Resources.get("data/textures/null_albedo.dds")->as<CTexture>();
  if (!textures[TS_NORMAL])
    textures[TS_NORMAL] = Resources.get("data/textures/null_normal.dds")->as<CTexture>();
  if (!textures[TS_METALLIC])
    textures[TS_METALLIC] = Resources.get("data/textures/black.dds")->as<CTexture>();
  if (!textures[TS_ROUGHNESS])
    textures[TS_ROUGHNESS] = Resources.get("data/textures/white.dds")->as<CTexture>();
  if (!textures[TS_EMISSIVE])
    textures[TS_EMISSIVE] = Resources.get("data/textures/black.dds")->as<CTexture>();

  std::string tech_name = j.value("technique", "objs.tech");
  tech = Resources.get(tech_name)->as<CTechnique>();

  category = category_names.valueOf(j.value("category", "solid").c_str());

  priority = j.value("priority", priority);

  casts_shadows = j.value("casts_shadows", casts_shadows);

  if (casts_shadows) {
    std::string shadows_mat_name = j.value("shadows_material", "data/materials/shadows.material");
    if( tech->usesSkin() ) 
      shadows_mat_name = j.value("shadows_material", "data/materials/shadows_skin.material");
    if (strcmp(tech_name.c_str(), "objs_culled_by_gpu.tech") == 0) {
      shadows_mat_name = j.value("shadows_material", "data/materials/shadows_culled_by_gpu.material");
    }
    shadows_material = Resources.get(shadows_mat_name)->as<CMaterial>();
  }

  cacheSRVs();

  return true;
}

void CMaterial::onFileChanged(const std::string& filename) {
  if (filename == getName())
    create(filename);
  else 
    // Maybe a texture has been updated, get the new shader resource view
    cacheSRVs();
}

void CMaterial::cacheSRVs() {
  // Maybe a texture has been updated, get the new shader resource view
  for (int i = 0; i < max_textures; ++i)
    srvs[i] = textures[i] ? textures[i]->getShaderResourceView() : nullptr;
}

void CMaterial::activate() const {
  tech->activate();
  if(ctes_material)
    ctes_material->activate();
  activateTextures(TS_ALBEDO);
}

void CMaterial::activateTextures(int slot0) const {
  Render.ctx->VSSetShaderResources(slot0, max_textures, (ID3D11ShaderResourceView**)srvs);
  Render.ctx->PSSetShaderResources(slot0, max_textures, (ID3D11ShaderResourceView**)srvs);
}

void CMaterial::renderInMenu() {
  category_names.debugInMenu("Category", category);
  ImGui::DragInt("Priority", &priority, 0.2f, 1, 200);
  ((CTechnique*)tech)->renderInMenu();
  ImGui::LabelText("Tech", "%s", tech->getName().c_str());

  for (int i = 0; i < max_textures; ++i) {
    const CTexture* t = textures[i];
    ImGui::LabelText(texture_slot_names.nth(i)->name, "%s", t->getName().c_str());
    if (t)
      ((CTexture*)t)->renderInMenu();
  }

  if(casts_shadows && shadows_material)
    ((CMaterial*)shadows_material)->renderInMenu();

}

void CMaterial::activateCompBuffers(TCompBuffers* c_buffers) const {
  const json& jdef = tech->getDefinition();
  if (jdef.count("gpu_buffers") > 0) {
    for (auto it : jdef["gpu_buffers"].items()) {
      assert(c_buffers);
      const std::string& key = it.key();
      const json& jvalue = it.value();
      auto gpu_buffer = c_buffers->getBufferByName(key.c_str());
      assert(gpu_buffer);
      int slot = jvalue.get<int>();
      Render.ctx->VSSetShaderResources(slot, 1, &gpu_buffer->srv);
      // If you want to use the buffers in GPU, use a slot number that do NOT
      // overlap with the texture slots, like 0, 1,2...16
      //Render.ctx->PSSetShaderResources(slot, 1, &gpu_buffer->srv);
    }
  }
  if (jdef.count("ctes_buffers") > 0) {
    for (auto it : jdef["ctes_buffers"].items()) {
      assert(c_buffers);
      const std::string& key = it.key();
      const json& jvalue = it.value();
      auto cte_buffer = c_buffers->getCteByName(key.c_str());
      cte_buffer->activate();
    }
  }
}