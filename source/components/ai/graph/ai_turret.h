#ifndef _AI_GOLEM
#define _AI_GOLEM

#include "ai_controller.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"



class CAITurret : public IAIController
{

	void IdleState(float dt);
	void AlignState(float dt);
	void AttackState(float dt);

	void ImpactState(float dt);
	void DeadState(float dt);

	template <typename T>
	void Send_DamageMessage(CEntity* entity, float dmg);
	
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

	//types
	bool isFixed;

	//laser -- cambiar a una subclase
	//bool isLaser = true;
	//float laserFrequecy = 0.5f;
	//float timeLaser = laserFrequecy;


	//timers
	float attackFrequecy = 0.5f;
	float timerAttack = attackFrequecy;

	float stunTimeMax = 1.5f;
	float stunTimer = 0.f;
	

	//ranges of vision
	float length_cone = 15.0f;
	float half_cone = 30.0f;
	float hearing_radius = 2.0f;
	float cone_vision = 10.f;
	float cone_vision_forget = 15.f;

	float forgetEnemyRange = 3.0;
	float forgetEnemy = length_cone + forgetEnemyRange;
	

	//gameplay parameters
	int damage = 10;
	int damageBullet = 15;
	int life = 100;
	float twistSpeed = 5.f;

	bool isView(float distance);
	void onCollision(const TMsgOnContact& msg);

};

#endif _AI_GOLEM