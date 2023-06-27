#include "mcv_platform.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_name.h"
#include "engine.h"
#include "comp_trigger_LUA.h"


DECL_OBJ_MANAGER("trigger_lua", TCompTriggerLua);

void TCompTriggerLua::debugInMenu() {
}

void TCompTriggerLua::load(const json& j, TEntityParseContext& ctx) {

	active = j.value("active", active);
	interval_enemy = j.value("interval_enemy", interval_enemy);
}


void TCompTriggerLua::registerMsgs()
{
	DECL_MSG(TCompTriggerLua, TMsgEntityTriggerEnter, onEnter);
	DECL_MSG(TCompTriggerLua, TMsgEntityTriggerExit, onExit);
	
}



void TCompTriggerLua::onExit(const TMsgEntityTriggerExit & msg) {


}

void TCompTriggerLua::onEnter(const TMsgEntityTriggerEnter & msg)
{
	/* TEST LUA
	Scripting.runScript(HEAL_PLAYER);
	//Scripting.runScript(RESTORE_MADNESS, "nothing");
	Scripting.runScript(GOD_MODE, "true");
	Scripting.runScript(SPAWN, PREFAB_SUSHI);

	if (activeEnemies) {
		Scripting.runScript(ACTIVE_ENEMIES, "true");
	}
	else {
		Scripting.runScript(ACTIVE_ENEMIES, "false");
	}
	activeEnemies = !activeEnemies;
	*/

	if (!active) { //solo entra aqui la primera vez
		active = true;
		killed_enemies = GameController.getEnemiesKilled();
	}

}

void TCompTriggerLua::update(float dt)
{
	if (active) {
		if (GameController.getEnemiesKilled() >= (killed_enemies + interval_enemy)) {
			//aqui ocurre algo, de momento spawnea enemigos
			for (int i = 0; i < interval_enemy; i++)
			{
				Scripting.runScript(SPAWN, PREFAB_SUSHI);
			}

			killed_enemies += interval_enemy;//update the enemies killed
		}

	}

}
