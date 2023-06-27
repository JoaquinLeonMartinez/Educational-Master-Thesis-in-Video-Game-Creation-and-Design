#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"


class TCompGrenadeGolemController : public TCompBase {

	
	VEC3  front;
	
	CHandle h_sender;
	
	bool flagExplota = false;

	int damage = 20;//esto cogerlo del golem 
	CHandle enemy;
	

	DECL_SIBLING_ACCESS();
	void onGrenadeInfoMsg(const TMsgAssignBulletOwner& msg);
	void onCollision(const TMsgOnContact& msg);
public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	static void registerMsgs();
	void launch();
	VEC3 getForceFrom(VEC3 fromPos, VEC3 toPos);
	TCompCollider* c_collider;
	float explosion_radius = 2.f;
	float fly_time = 1.5f;
	//bool flagColision = false; //quizas cambiar
	//
	//float final_time_explosion = 10.f;//cambiar
	//float init_time_explosion = final_time_explosion;
};