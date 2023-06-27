#ifndef _AI_BOX
#define _AI_BOX

#include "ai_controller.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"


class CAIBox : public IAIController
{

	void DeadState(float dt);
	void ImpactState(float dt);
	void IdleState(float dt);
	

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
	int life = 2000;
	int amount = 20;
	void onCollision(const TMsgOnContact& msg);
    void onDamage(const TMsgDamageToEnemy& msg);



};

#endif _AI_BOX