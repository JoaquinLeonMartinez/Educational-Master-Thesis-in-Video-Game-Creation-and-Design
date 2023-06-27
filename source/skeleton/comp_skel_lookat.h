#ifndef INC_COMP_SKEL_LOOKAT_H_
#define INC_COMP_SKEL_LOOKAT_H_

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "bone_correction.h"

class CalModel;
struct TCompSkeleton;

struct TCompSkelLookAt : public TCompBase {
  CHandle     h_skeleton;        // Handle to comp_skeleton of the entity being tracked
  CHandle     h_target_entity;
  std::string target_entity_name;

  VEC3        target;
  float       amount = 0.f;
  float       target_transition_factor = 0.95f;
  float      constantTurn = 0.3f;

  float timeTurn = 0.f;
  bool flagFirst = false;
  float angleMax = 180;

  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();
  void renderDebug();
  DECL_SIBLING_ACCESS();
};

#endif
