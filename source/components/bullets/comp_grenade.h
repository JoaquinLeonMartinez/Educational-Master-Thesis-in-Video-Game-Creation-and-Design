#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"


class TCompGrenadeController : public TCompBase {


	VEC3  front;
	CHandle h_sender;
	boolean flagExplota = false;
	float explosion_radius_hit = 7.f;
	float intensityDamage = 20.f;
	float mass = 0.75;
  float force = 10.0f;
  float height_force = 2.0f;

	DECL_SIBLING_ACCESS();
	void onGrenadeInfoMsg(const TMsgAssignBulletOwner& msg);
	void onCollision(const TMsgOnContact& msg);
public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	static void registerMsgs();
  void renderDebug();
};