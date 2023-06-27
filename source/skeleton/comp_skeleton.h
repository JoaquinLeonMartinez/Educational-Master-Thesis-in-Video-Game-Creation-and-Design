#ifndef INC_COMPONENT_SKELETON_H_
#define INC_COMPONENT_SKELETON_H_

#include "geometry/geometry.h"
#include "components/common/comp_base.h"
#include "entity/entity.h"

class CGameCoreSkeleton;
class CalModel;

struct TCompSkeleton : public TCompBase {

  TCompSkeleton();
  ~TCompSkeleton();

  std::vector<int> feetBonesId;
  int actualCycleAnimId[2];
  std::map<std::string, int> stringAnimationIdMap;
  float lastFrameCyclicAnimationWeight;
  float cyclicAnimationWeight;

  CalModel*                 model = nullptr;
  CCteBuffer<TCteSkinBones> cb_bones;

  void updateCtesBones();
  void renderDebug();
  void debugInMenu();
  void update(float dt);
  void load(const json& j, TEntityParseContext& ctx);

  bool _unscaledTime = false;

  void changeCyclicAnimation(int anima1Id, float speed = 1.0f, int anima2Id = -1, float weight = 1.0f, float in_delay = 0.10f, float out_delay = 0.10f);
  void executeActionAnimation(int animaId, float speed = 1.0f, float in_delay = 0.10f, float out_delay = 0.10f);

  void setCyclicAnimationWeight(float new_value);
  float getCyclicAnimationWeight();
  int getAnimationIdByName(std::string animaName);
  bool actionAnimationOnExecution();
  bool isExecutingCyclicAnimation(int animaId);
  bool isExecutingActionAnimation(std::string animaName);
  float getAnimationDuration(int animaId);
  VEC3 getBonePositionByName(const std::string & name);
  VEC3 setBonePositionByName(const std::string & name);
  VEC3 getBonePositionById(int id);
  void setBonePositionById(int id, VEC3 position);
  void removeAnimationByNBame(std::string animName);
  void clearAnimations();
  DECL_SIBLING_ACCESS();
};


#endif
