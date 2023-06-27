#ifndef _AI_GOLEM
#define _AI_GOLEM

#include "ai_controller.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"



class CAIGolem : public IAIController
{

	void IdleState(float dt);
	void AlignState(float dt);
	void ThrowState(float dt);
	void MeleeState(float dt);
	void BatteryEfectState(float dt);

	void ImpactState(float dt);
	void DeadState(float dt);

	template <typename T>
	void Send_DamageMessage(CEntity* entity, float dmg);
	void onGravity(const TMsgGravity& msg);

	CHandle h_player;

	DECL_SIBLING_ACCESS();
public:
	void Init();
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	void renderDebug();
	static void registerMsgs();

	CHandle h_sender;
	CHandle h_bullet;

private:

	//external effects
	VEC3 battery_position = VEC3();
	float battery_time = 0.f;
	float batterySpeed = 20.f;

	//timers
	float delay = 1.0f;
	float timer = delay;
	float timerFinishAttack = 0.3f;
	float timerFinishAttackAux = 0.3f;

	float throwFrequecy = 5.0f;
	float timerGrenade = throwFrequecy;

	float meleeFrequency = 1.0f;
	float timerMelee = meleeFrequency;

	//probabilities
	float attackProb = 90.f;

	//ranges of vision
	float length_cone = 30.0f;
	float half_cone = 150.0f;
	float hearing_radius = 2.0f;
	float cone_vision = 10.f;
	float cone_vision_forget = 15.f;

	float forgetEnemyRange = 3.0;
	float forgetEnemy = length_cone + forgetEnemyRange;


	//gameplay parameters
	int damage = 10;
	int damageBullet = 15;
	float damageArea = 1.0;
	int life = 10;
	float speedAttack = 15.0f;
	float meleeRange = 7.0f;
	bool attacking = false;
	float twistSpeed = 5.f;

	bool isView(float distance);
	void onCollision(const TMsgOnContact& msg);

	int count;
	int countImpact = 300;

};

#endif _AI_GOLEM