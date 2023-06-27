#include "mcv_platform.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_name.h"
#include "engine.h"
#include "comp_trigger_checkpoint.h"

DECL_OBJ_MANAGER("trigger_checkpoint", TCompTriggerCheckpoint);

/*

Purpose:

This component serves as a handler for the onEnter event. 
It should therefore be placed inside an entity which contains a trigger meant to be a checkpoint.

Behavior:

Once the onEnter event is triggered, this component checks whether it's already been activated.
If it hasn't, it checks whether it was the player who triggered it.
If it was, it calls GameController.saveCheckpoint with the player's position and rotation, and marks the checkpoint as activated.

*/

void TCompTriggerCheckpoint::debugInMenu() {
}

void TCompTriggerCheckpoint::load(const json& j, TEntityParseContext& ctx) {
}

void TCompTriggerCheckpoint::registerMsgs()
{
	DECL_MSG(TCompTriggerCheckpoint, TMsgEntityTriggerEnter, onEnter);
}

void TCompTriggerCheckpoint::onEnter(const TMsgEntityTriggerEnter & msg)
{
    //Once the onEnter event is triggered, this component checks whether it's already been activated.
    if (!used) {
		CHandle h_player = EngineEntities.getPlayerHandle();
    //If it hasn't, it checks whether it was the player who triggered it.
    if (h_player == msg.h_entity) {
      //If it was, it calls GameController.saveCheckpoint with the player's position and rotation, and marks the checkpoint as activated.
      TCompName* cname = get<TCompName>();
			dbg("Checkpoint \"%s\" activated.\n", cname->getName());
			CEntity* e_player = h_player;
			TCompTransform* tPlayerPos = e_player->get<TCompTransform>();
			TCompTransform* myPos = get<TCompTransform>();
		  GameController.saveCheckpoint(VEC3(myPos->getPosition().x, tPlayerPos->getPosition().y, myPos->getPosition().z), tPlayerPos->getRotation());
			used = true;
		}
	}
}

void TCompTriggerCheckpoint::update(float dt)
{

}
