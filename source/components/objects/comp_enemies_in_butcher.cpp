#include "mcv_platform.h"
#include "comp_enemies_in_butcher.h"
#include "components/controllers/character/comp_character_controller.h"
#include "modules/game/module_game_controller.h"
#include "components/objects/comp_platform_fire_trap.h"
#include "engine.h"

DECL_OBJ_MANAGER("enemies_in_butcher", TCompEnemiesInButcher);

void TCompEnemiesInButcher::debugInMenu() {
  
}


void TCompEnemiesInButcher::registerMsgs() {
	DECL_MSG(TCompEnemiesInButcher, TMSgEnemyDead, onSushiTrap);
}


void TCompEnemiesInButcher::load(const json& j, TEntityParseContext& ctx){
	
}


void TCompEnemiesInButcher::update(float dt) {
	
	if (count >= 3) {
		CHandle handle = GameController.entityByName("Platform Fire Particles");
		CEntity* entity = (CEntity*)handle;
		if (entity != nullptr) {
			TCompFirePlatformTrap* firePlatformTrap = entity->get<TCompFirePlatformTrap>();
			firePlatformTrap->flagActivate = false;
			Scripting.execActionDelayed("cinematica_fire_off()", 0);
			Scripting.execActionDelayed("activeElevatorInitCarniceria_player()", 4.0);
			GameController.destroyCHandleByName("enemies_in_butcher");
		}
		
	}
}

void TCompEnemiesInButcher::onSushiTrap(const TMSgEnemyDead & msg) {
	CHandle handle = msg.h_entity;
	CEntity* entity = (CEntity*) handle;
	TCompName* name = entity->get<TCompName>();
	std::string nameEntity = name->getName();
	if(nameEntity.compare("Sushi000") == 0 || nameEntity.compare("Ranged_Sushi_0") == 0) {//los de nombre diferente  son los suishi de las plantas de arriba
		count++;
	}


}

