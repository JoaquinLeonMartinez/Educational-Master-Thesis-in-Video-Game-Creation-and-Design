#pragma once

#include "comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
 
class CMesh;
class CMaterial;

struct TCompRender : public TCompBase {

  struct MeshPart {
    const CMesh*      mesh = nullptr;
    const CMaterial*  material = nullptr;
    int               mesh_group = 0;
    int               mesh_instances_group = 0;
    bool              is_visible = true;
    int               state = 0;          // user defined...
  };

  AABB                    aabb;
  bool                    is_visible = true;
  CHandle                 h_transform;
  VEC4                    color;
  std::vector< MeshPart > parts;
  int                     curr_state = 0;
  void readMesh(const json& j);
  void onDefineLocalAABB(const TMsgDefineLocalAABB& msg);
  void renderDebug();

public:
  ~TCompRender();
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void showMeshesWithState(int new_state);
  void setMaterial(const std::string &name, bool unMaterial);
  void updateRenderManager();

  static void registerMsgs();

  DECL_SIBLING_ACCESS();
};