#include "mcv_platform.h"
#include "ai_box.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/john/comp_bullet.h"
#include "components/common/comp_tags.h"
#include "time.h"
#include "stdlib.h"
#include "modules/module_physics.h"

#include "modules/module_physics.h"

#include "PxPhysicsAPI.h"


using namespace physx;


DECL_OBJ_MANAGER("ai_box_destroyable", CAIBox);

void CAIBox::Init()
{

	
	

	AddState("DEAD", (statehandler)&CAIBox::DeadState);
	AddState("IMPACT", (statehandler)&CAIBox::ImpactState);

	AddState("IDLE", (statehandler)&CAIBox::IdleState);
	ChangeState("IDLE");
		
}



void CAIBox::ImpactState(float dt) {



	ChangeState("DEAD");
}

void CAIBox::IdleState(float dt) {
	if (!h_player.isValid()) {
		h_player = getEntityByName("Player");
		return;
	}
}


void CAIBox::DeadState(float dt) {

	CEntity* e_player = (CEntity *)h_player;
	//dbg("recibe el impacto");
	TMsgMunition msg;
	msg.amount = amount;
	e_player->sendMsg(msg);

	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();
}



void CAIBox::load(const json& j, TEntityParseContext& ctx) {
	this->Init();
	
	
}

void CAIBox::debugInMenu() {
	
}

void CAIBox::renderDebug() {
	
}



void CAIBox::onCollision(const TMsgOnContact& msg) {
	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	if (source_of_impact) {
		TCompTags* c_tag = source_of_impact->get<TCompTags>();
		if (c_tag) {
			std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
			if (strcmp("grenade", tag.c_str()) == 0) {
				ChangeState("DEAD");

			} 
			else if (strcmp("shrapnel", tag.c_str()) == 0) {
				
				ChangeState("DEAD");

			}
			else if (strcmp("bullet", tag.c_str()) == 0) {
				
				ChangeState("DEAD");

			}
			
		}
	}
}

void CAIBox::onDamage(const TMsgDamageToEnemy& msg) {//??
  h_sender = msg.h_sender;
  h_bullet = msg.h_bullet;
  life -= msg.intensityDamage;
  if (life < 0)
    life = 0;
  ChangeState("IMPACT");
}

void CAIBox::registerMsgs() {
	DECL_MSG(CAIBox, TMsgOnContact, onCollision);
  DECL_MSG(CAIBox, TMsgDamageToEnemy, onDamage);
  
}
