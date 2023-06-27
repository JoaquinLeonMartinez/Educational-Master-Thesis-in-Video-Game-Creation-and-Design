#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "PxPhysicsAPI.h"




namespace physx {
  class PxRigidActor;
}

class TCompCollider : public TCompBase {
public:
  DECL_SIBLING_ACCESS();

  json jconfig;

  ~TCompCollider();

  physx::PxRigidActor*        actor = nullptr;
  physx::PxCapsuleController* controller = nullptr;
  CMesh*                      debug_mesh = nullptr;

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  void onEntityCreated(const TMsgEntityCreated& msg);

  typedef void (TCompCollider::*TShapeFn)(physx::PxShape* shape, physx::PxGeometryType::Enum geometry_type, const void* geom, MAT44 world);
  void renderDebugShape(physx::PxShape* shape, physx::PxGeometryType::Enum geometry_type, const void* geom, MAT44 world);
  void debugInMenuShape(physx::PxShape* shape, physx::PxGeometryType::Enum geometry_type, const void* geom, MAT44 world);
  void onEachShape(TShapeFn fn);
  void setGlobalPose(VEC3 pos, QUAT rot);

  static void registerMsgs();

};