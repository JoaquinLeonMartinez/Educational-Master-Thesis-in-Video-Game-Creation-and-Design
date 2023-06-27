#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"


class TCompBlackboard : public TCompBase {
	DECL_SIBLING_ACCESS();

public:

	/* GUIA DE FUNCIONAMIENTO EXPRESS
	- La blackboard tiene una serie de slots disponibles para los enemigos (maxSlotsAttacking)
	- Cada enemigo que este atacando ocupa un numero de slots,
	de momento el cupcake ocupa uno (numSlotsCupcake = 1) y el sushi dos (numSlotsSushi = 2)
	- Cuando un enemigo quiere atacar pregunta si hay slots con checkPermission y se envia un mensaje al enemigo con la respuesta 
	- Cuando un enemigo pierde de vista al player o muere libera los slots que ocupa (forgetPlayer())
	- cuando el player muere avisa a la blackboard par que no deje a ningun enemigo atacar y se olvide del player (playerIsDeath)

	NOTA: de momento los slots solo funcionan con cupcakes pero cuando definamos bien lo que hace el sushi en lugar de atacar,
	la blackboard esta preparada para gestionarlo

	Para cualquier duda preguntar a Joaquin :D
	*/

	void load(const json& j, TEntityParseContext& ctx);
	bool checkPermission(CHandle _enemy, EntityType type);
	void forgetPlayer(CHandle _enemy, EntityType type);
	void playerIsDeath(bool player_death);
	bool alreadyAttacking(CHandle _enemy, EntityType type);
	bool getPlayerDeath();
	void update(float delta);
	void resetSlots();
	void resetBlackBoard();
	//void incrementTermostateBroken();

	//int getNumTermostatesBroken();



	//std::vector<CEntity*> enemiesAttacking;//¿?
	std::list<CHandle> cupcakeAttacking;
	std::list<CHandle> sushiAttacking;

private:
	
	int numTermostateBroken = 0;
	int maxSlotsAttacking = 10; //maximo de slots que te pueden atacar
	int slotsAttacking = 0;
	int numSlotsCupcake = 1; //slots que ocupa un cupcake
	int numSlotsSushi = 2; //slots que ocupa un sushi

	
	float maxTimerResetEnemies= 4.0f;
	float timerResetEnemies= maxTimerResetEnemies;

	bool playerDeath = false;

};