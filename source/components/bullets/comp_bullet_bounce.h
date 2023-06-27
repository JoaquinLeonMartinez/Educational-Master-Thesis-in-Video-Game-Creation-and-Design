#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"

class TCompBulletBounceController : public TCompBase {

	float speed = 20.0f;
	VEC3  front;
	float collision_radius = 2.f;
	CHandle h_sender;
	float intensityDamage = 20.0f;
    int bounces = 3;//1
	float limitTime = 10.f;

	
	DECL_SIBLING_ACCESS();
	void onBulletInfoMsg(const TMsgAssignBulletOwner& msg);
  void onCollision(const TMsgOnContact& msg);

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	static void registerMsgs();
};