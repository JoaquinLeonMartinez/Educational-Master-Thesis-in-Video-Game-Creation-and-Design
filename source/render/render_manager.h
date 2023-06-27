#pragma once

#include "render/textures/material.h"

class CMaterial;
class CMesh;

class CRenderManager {

public:
  // Normal
  struct TNormalKey {
    const CMesh*     mesh = nullptr;
    const CMaterial* material = nullptr;
    CHandle          h_owner;           // to comp_render
    CHandle          h_transform;       // World
    CHandle          h_aabb;            // Abs if any
    uint16_t         submesh_id = 0;
    uint16_t         instanced_group_id : 14;
    uint16_t         is_instanced : 1;
    uint16_t         uses_custom_buffers : 1;
    void renderInMenu();
  };

  static bool sortNormalKeys(const TNormalKey& k1, const TNormalKey& k2);

private:
  template< typename TKey >
  struct TKeyGroup {
    std::vector< TKey > keys;
    bool                sort_required = false;
    void add(const TKey& new_key) {
      keys.push_back(new_key);
      sort_required = true;
    }

    void del(CHandle h_owner) {
      auto it = keys.begin();
      while (it != keys.end()) {
        if (it->h_owner == h_owner)
          it = keys.erase(it);
        else
          ++it;
      }
    }

    void renderInMenu() {
      auto it = keys.begin();
      while (it != keys.end()) {
        ImGui::PushID(&(*it));
        
        it->renderInMenu();

        ImGui::SameLine();
        if (ImGui::SmallButton("X"))
          it = keys.erase(it);
        else
          ++it;

        ImGui::PopID();
      }
    }
  };

  TKeyGroup< TNormalKey > normal_keys;
  CHandle                 h_camera;

  std::map< eRenderCategory, uint32_t > draw_calls_per_category;

public:
  static CRenderManager& get();

  void addKey(
    CHandle          h_owner,
    const CMesh*     mesh,
    const CMaterial* material,
    CHandle          h_transform,
    uint16_t         submesh_id,
    uint16_t         instanced_group_id = 0
  );

  void delKeys(CHandle h_transform);
  void render(eRenderCategory category);
  void renderInMenu();

  void setEntityCamera(CHandle h_new_entity_camera) {
    h_camera = h_new_entity_camera;
  }
  CHandle getEntityCamera() const { return h_camera; }
};
