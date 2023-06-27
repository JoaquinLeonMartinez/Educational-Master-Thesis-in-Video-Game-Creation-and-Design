#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "PxPhysicsAPI.h"

namespace physx {
	class PxCapsuleController;
}

class TCompCharacterCollider : public TCompBase {
public:
  DECL_SIBLING_ACCESS();

  struct TConfig {
    physx::PxGeometryType::Enum geometryType = physx::PxGeometryType::eINVALID;
    VEC3 planeNormal;
    float planeDistance;
    VEC3 halfExtent;
    float radius = 0.f;
    float density = 1.0f;
    bool isDynamic = false;
    bool isTrigger = false;
  };

  json jconfig;

  TConfig config;

  physx::PxCapsuleController* gController = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void onEntityCreated(const TMsgEntityCreated& msg);
  static void registerMsgs();

};