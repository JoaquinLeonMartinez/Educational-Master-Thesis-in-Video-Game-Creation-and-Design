#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
 
struct SimpleVertex
{
  VEC3 Pos;
  VEC4 Color;
  SimpleVertex() = default;
  SimpleVertex(VEC3 newPos, VEC4 newColor) : Pos(newPos), Color(newColor) {}
};

class TCompTrail :  public TCompBase {
  DECL_SIBLING_ACCESS();

  //initial mesh data
  const std::vector<SimpleVertex> vtxs = {
    { VEC3(0, 0, 0), VEC4(1, 1, 1, 1) }
  , { VEC3(1, 0, 0), VEC4(1, 1, 1, 1) }
  , { VEC3(0, 1, 0), VEC4(1, 1, 1, 1) }
  , { VEC3(1, 1, 0), VEC4(1, 1, 1, 1) }
  };

  CHandle target;
  CMesh* trail_mesh;
  void onCreation(const TMsgEntityCreated& msg);

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);
    static void registerMsgs();
};