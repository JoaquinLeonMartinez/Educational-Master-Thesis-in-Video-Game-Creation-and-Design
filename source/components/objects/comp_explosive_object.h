#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/common/comp_transform.h"

class TCompExplosiveObject : public TCompBase {
	DECL_SIBLING_ACCESS();

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	static void registerMsgs();

	void enable();

private:
	bool _hasExploded = false;
	bool _isEnabled = false;
	bool _isFireEnabled = false;

	float _explosionForce = 120.f;
	float _explosionDamage = 20.f;
	float _explosionRadius = 6.f;
	float _explosionDelay = 3.f;
	float _explosionTimer = _explosionDelay;

	float _fireAreaDuration = 6.f;

	float _delayForExternalExplosion = 0.3f;

	VEC3 _position = VEC3(20,0,0);
	
	bool _sphereEnabled = false;
	float _sphereDuration = .5f;
	float _sphereTimer = _sphereDuration;

	void explode();

	void onDamageInfoMsg(const TMsgDamage& msg);
	void onTriggerEnter(const TMsgEntityTriggerEnter& msg);
	void onTriggerExit(const TMsgEntityTriggerExit& msg);

	std::vector<CEntity*> candidates;
};