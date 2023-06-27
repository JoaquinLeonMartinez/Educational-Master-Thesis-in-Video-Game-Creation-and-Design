#include "engine.h"
#include "modules/module_physics.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"

using namespace physx;

class TCompAimController : public TCompBase {

  CModulePhysics::FilterGroup filter_group;
  CModulePhysics::FilterGroup filter_mask;
  PxFilterData                filter_data;

  VEC3                        ray_origin;
  VEC3                        ray_front;
  VEC4                        constants;

  // https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/SceneQueries.html
  bool                        hit_status = false;
  PxRaycastBuffer             hit;
  PxQueryFilterData           query_filter_data;
  float                       max_distance = 200.0f;

  static const PxU32          bufferSize = 256;        // [in] size of 'hitBuffer'
  PxRaycastHit                hitBuffer[bufferSize];   // [out] User provided buffer for results
  
  DECL_SIBLING_ACCESS();

  void updateFilterData();

public:
  float closest = 200.f;
  VEC3 closest_impact = VEC3();
  VEC3 closest_normal = VEC3();

  void debugInMenu();  
  void load(const json& j, TEntityParseContext& ctx);
  static void registerMsgs();
  void renderDebug();
  void update(float dt);

};

