#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"
class TCompKinematicBullet : public TCompBase {


	DECL_SIBLING_ACCESS();
	void onLaunch(const TMsgAssignBulletOwner& msg);
	void onCollision(const TMsgEntityTriggerEnter& msg);


public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void destroy();
	void renderDebug();
	void debugInMenu();
	static void registerMsgs();

private:
  bool comicfeed = false;
  bool _destroyOnCollission = true;
  bool _requestAudioPermission = false;
  bool _isEnabled = true;
	float _speed = 17.f;
  void expansiveWave();
	VEC3 _targetDirection = VEC3().Zero;
	TMsgDamage _messageToTarget;
  EntityType targetType;
  std::string _audioOnHit = "";
};

