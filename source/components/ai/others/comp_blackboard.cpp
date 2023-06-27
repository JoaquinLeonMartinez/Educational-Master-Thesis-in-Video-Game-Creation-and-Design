#include "mcv_platform.h"
#include "engine.h"
#include "comp_blackboard.h"
#include "components/common/comp_tags.h"
#include "modules/game/module_game_controller.h"
#include "components/controllers/character/comp_character_controller.h"

DECL_OBJ_MANAGER("comp_blackboard", TCompBlackboard);


void TCompBlackboard::load(const json& j, TEntityParseContext& ctx) {
	maxSlotsAttacking = j.value("maxSlotsAttacking", maxSlotsAttacking);
	slotsAttacking = j.value("slotsAttacking", slotsAttacking);
	numSlotsCupcake = j.value("numSlotsCupcake", numSlotsCupcake);
	numSlotsSushi = j.value("numSlotsSushi", numSlotsSushi);
}
/*
int TCompBlackboard::getNumTermostatesBroken() {
	return numTermostateBroken;
}

void TCompBlackboard::incrementTermostateBroken() {
	numTermostateBroken++;

	if (numTermostateBroken == 1) {//esto lo suyo seria hacerlo en el update
		//Scripting.runScript(ACTIVE_SUSHIS, "true");
		Scripting.runScript("active_all_melee_sushi", "true");

	}
	else if (numTermostateBroken == 2) {
		//Scripting.runScript(ACTIVE_ENEMIES, "false"); //esto en principio los para
		Scripting.runScript("on_active_enemies", "false");

		CHandle h_player = GameController.getPlayerHandle();
		CEntity* e_player = (CEntity*)h_player;
		TCompCharacterController* player = e_player->get<TCompCharacterController>();
		player->endGame = true;

		//GameController.getPlayerHandle().get<TCompCharacterController>endGame = true;
		//GameController.stopBehaviorTrees();//mejor matar todos??
	}
}
*/
void TCompBlackboard::resetBlackBoard() {
	slotsAttacking = 0;
	cupcakeAttacking.clear();
	sushiAttacking.clear();
	playerIsDeath(false);
}

void TCompBlackboard::playerIsDeath(bool player_death) {

	playerDeath = player_death;

	TMsgBlackboard msg_out;
	msg_out.player_dead = playerDeath;

	//send stop message to all enemies
	VHandles handles_in_scene = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (auto& instance : handles_in_scene) {
		instance.sendMsg(msg_out);
	}

}

void TCompBlackboard::forgetPlayer(CHandle _enemy, EntityType type) { //este metodo es llamado por el enemigo al morir para que liberen sus slots
	std::list <CHandle> ::iterator it;
	if (type == CUPCAKE) {
		for (it = cupcakeAttacking.begin(); it != cupcakeAttacking.end(); ++it) {
			if ((*it) == _enemy) {
				cupcakeAttacking.erase(it);
				slotsAttacking -= numSlotsCupcake;
				break;
			}
		}
	}
	else if (type == SUSHI) {
		for (it = sushiAttacking.begin(); it != sushiAttacking.end(); ++it) {
			if ((*it) == _enemy) {
				sushiAttacking.erase(it);
				slotsAttacking -= numSlotsSushi;
				break;
			}
		}

	}


}

void TCompBlackboard::resetSlots() {
	CEntity* enemy;
	std::list <CHandle> ::iterator it;
	for (it = cupcakeAttacking.begin(); it != cupcakeAttacking.end(); ++it) {

		TMsgBlackboard msg_out;
		msg_out.is_posible = false;
		msg_out.player_dead = playerDeath;
		((CEntity*)(*it))->sendMsg(msg_out);
		slotsAttacking -= numSlotsCupcake;
	}
	cupcakeAttacking.clear();

	for (it = sushiAttacking.begin(); it != sushiAttacking.end(); ++it) {

		TMsgBlackboard msg_out;
		msg_out.is_posible = false;
		msg_out.player_dead = playerDeath;
		((CEntity*)(*it))->sendMsg(msg_out);
		slotsAttacking -= numSlotsSushi;
	}
	sushiAttacking.clear();
}

bool TCompBlackboard::alreadyAttacking(CHandle _enemy, EntityType type) {

	std::list <CHandle> ::iterator it;
	if (type == CUPCAKE) {
		for (it = cupcakeAttacking.begin(); it != cupcakeAttacking.end(); ++it) {
			if ((*it) == _enemy) {
				return true;
			}
		}
	}
	else if (type == SUSHI) {
		for (it = sushiAttacking.begin(); it != sushiAttacking.end(); ++it) {
			if ((*it) == _enemy) {
				return true;
			}
		}
	}

	return false;
}

bool TCompBlackboard::getPlayerDeath() {
	return playerDeath;
}

bool TCompBlackboard::checkPermission(CHandle _enemy, EntityType type) {
	CEntity* enemy = _enemy;
	TMsgBlackboard msg_out;

	if (alreadyAttacking(_enemy, type)) { //no es necesario hacer esta comprobacion, al menos con el cupcake
		msg_out.is_posible = true;
	}
	else {
		if (type == CUPCAKE) {
			if ((maxSlotsAttacking - slotsAttacking) >= numSlotsCupcake) {
				slotsAttacking += numSlotsCupcake;
				cupcakeAttacking.push_back(enemy);
				msg_out.is_posible = true;
			}
			else {
				msg_out.is_posible = false;
			}
		}
		else if (type == SUSHI) {
			if ((maxSlotsAttacking - slotsAttacking) >= numSlotsSushi) {
				slotsAttacking += numSlotsSushi;
				sushiAttacking.push_back(enemy);
				msg_out.is_posible = true;
			}
			else {
				msg_out.is_posible = false;
			}
		}
	}

	msg_out.player_dead = playerDeath;
	enemy->sendMsg(msg_out);
	return msg_out.is_posible;
}


void TCompBlackboard::update(float delta) {
	/*
	if (timerResetEnemies <= 0) {
		timerResetEnemies = maxTimerResetEnemies;
		//resetSlots();

	}
	else {
		timerResetEnemies -= delta;
	}
	*/
}

