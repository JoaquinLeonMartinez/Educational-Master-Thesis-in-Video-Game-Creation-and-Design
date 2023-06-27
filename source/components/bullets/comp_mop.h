#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"


class TCompMopController : public TCompBase {
	float radius_hit = 2.f;
	float max_distance_ray = 10.0f;
	float intensityDamage = 20;
	VEC3  front;
	CHandle h_sender;
	DECL_SIBLING_ACCESS();
	void onMopInfoMsg(const TMsgAssignBulletOwner& msg);
	void onCollision(const TMsgOnContact& msg);
	bool flagColision = false;
public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	static void registerMsgs();
	TCompCollider* c_collider;
	TCompTransform* c_trans;
	
};