#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/ai/graph/ai_controller.h"
#include "modules/module_physics.h"

class TCompTeleport : public TCompBase
{
  DECL_SIBLING_ACCESS();
public:
  //void update(float dt);
  void load(const json& j, TEntityParseContext& ctx);
  //void debugInMenu();
  void rayCast();
  void renderDebug();
  void update(float delta);
  bool canCombo();
  bool comboDone = false;

  float timeAfterTeleport = 0.f;
  float windowTimeCombo = 2.f;
  
  float timeAfterTeleportUI = 0.5f;
  CHandle h_camera;
  CHandle h_player;

  //DEBUG
  VEC3 objective;
  VEC3 source;

private:
  void teleportWithEnemy(VEC3 candidate_position);
  void teleportWithObject(VEC3 candidate_position);
  void manageCameras();
  bool hasCollided(bool colDetected, physx::PxRaycastBuffer hit);
};

