#ifndef _AI_PATROL_SOLDIER
#define _AI_PATROL_SOLDIER

#include "ai_controller.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"


class CAIPatrol_Soldier : public IAIController
{

	void SeekwptState(float dt);
	void NextwptState(float dt);
	void ChaseState(float dt);
	void IdlWarState(float dt);
	void OrbitLeftState(float dt);
	void OrbitRightState(float dt);
	void ImpactState(float dt);
	void DeadState(float dt);

	CHandle h_player;

	DECL_SIBLING_ACCESS();
	void onDamageInfoMsg(const TMsgDamage& msg);
public:
	void Init();
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	void renderDebug();
	static void registerMsgs();

	CHandle h_sender;
	CHandle h_bullet;
	
	int damage;
	int life = 100;
	float speed = 5.0f;
private:
	std::vector<VEC3> positions;
	int i = 0;
	VEC3 position;
	VEC3 currentPosition;
	VEC3 nextPoint;
	float half_cone = 30.0f;
	float length_cone = 3.0f;
	float hearing_radius = 2.0f;
	bool isView();

	int count;
	int countImpact = 300;

};

#endif _AI_PATROL_SOLDIER