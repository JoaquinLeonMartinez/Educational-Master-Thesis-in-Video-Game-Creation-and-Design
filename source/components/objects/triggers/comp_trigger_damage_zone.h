#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

class TCompTriggerDamageZone : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  int damage = 999;
  float damageInterval = 0.1f;
  float damageTimer = damageInterval; //esto hace que la primera vez no tenga que esperar
  static void registerMsgs();
  bool active = true;
 
  //CHandle* candidate;
  //  std::vector<Vector3> positions;
  std::vector<CEntity*> candidates;

private:
  void onEnter(const TMsgEntityTriggerEnter & msg);
  void onExit(const TMsgEntityTriggerExit & msg); 
  void onActiveMsg(const TMsgSetActive & msg);
};

