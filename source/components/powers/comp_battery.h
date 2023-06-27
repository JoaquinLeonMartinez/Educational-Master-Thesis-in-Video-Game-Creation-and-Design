#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"
#include "modules/game/audio/audioEvent.h"

class TCompBatteryController : public TCompBase {


	DECL_SIBLING_ACCESS();
	void onBatteryInfoMsg(const TMsgAssignBulletOwner& msg);
  void onCollision(const TMsgOnContact& msg);

	CHandle h_sender;
	bool flagColisiona = false;
	TCompCollider* colliderAmover;
	float timeEffect = 5.f;
	float distance = 1.5f;
	float distanceFire = 1.f;
	float atractionForce = 20.f;
	float battery_radius_hit = 8.f;
	float battery_radius_trajectory_hit = 0.1f;
	float mass = 100.f;
	float force = 10.0f;
	float height_force = 8.0f;
	float limitTime = timeEffect + 5.0f;
	float gravityOriginHeight = 3.f;
	float velocityCurve = 20.f;
  bool startedEffect = false;
	float max_distance_enemy = 13.f;

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void renderDebug();
	void debugInMenu();
	void shoot(VEC3 front);
	static void registerMsgs();
	void onBatteryFire(const TMsgDamage & msg);
	
	TCompTransform* c_trans;

private:
	std::vector<VEC3> _knots;
	int i = 0;
	//fire atributes
	float fireTimeMax = 0.8f;
	float fireTime = fireTimeMax;
	float fireDamage = 3.f;
	bool batteryFire = false;
	VEC3 nextPoint;
	bool isScenario = false;
  bool isKinematic = true;
  std::list<CHandle> enemiesCatchBattery;
  std::list<CHandle> enemiesBolt;
  int maxEnemiesAffected = 5;
  float pulseTimer = 0;
  float pulseDelay = 0.75f;
	bool firstCollision = true;

  AudioEvent audioEffect;
};

