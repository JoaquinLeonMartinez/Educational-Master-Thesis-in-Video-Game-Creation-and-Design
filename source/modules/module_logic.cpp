#include "mcv_platform.h"
#include "module_logic.h"
#include "engine.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "components/common/comp_tags.h"
#include "components/common/comp_transform.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/physics/comp_rigid_body.h"
#include "module_physics.h"

bool CModuleLogic::start()
{
  //init scripts LUA
  return true;
}

void CModuleLogic::stop()
{
	//init LUA scripts
}
void CModuleLogic::update(float dt)
{
	//update LUA scripts
}
void CModuleLogic::renderInMenu()
{

}
//Functions for LUA
//...

//Auxiliar Fabrica Functions 
CModuleLogic* getLogic() { 
		return EngineLogic.getPointer(); 
}


CHandle CModuleLogic::spawnBot(VEC3 &position, const VEC3 & lookat,std::string &type) {
	TEntityParseContext ctxSpawn;
	parseScene("data/prefabs/" + type , ctxSpawn);
	CHandle h = ctxSpawn.entities_loaded[0];
	CEntity* e = h;
	TCompTransform* e_pos = e->get<TCompTransform>();
	e_pos->lookAt(position, lookat);
	return h;
}


void CModuleLogic::move(const std::string & name, const VEC3 & pos, const VEC3 & lookat)
{
	CHandle h_to_move = getEntityByName(name);
	if (h_to_move.isValid()) {
		CEntity* e_to_move = h_to_move;
		TCompTransform* e_pos = e_to_move->get<TCompTransform>();
		e_pos->setPosition(pos);
		TCompCollider* e_collider = e_to_move->get<TCompCollider>();
		TCompRigidBody* e_rigidbody = e_to_move->get<TCompRigidBody>();
		physx::PxExtendedVec3 pos = physx::PxExtendedVec3(e_pos->getPosition().x, e_pos->getPosition().y, e_pos->getPosition().z); //VEC3_TO_PXVEC3(e_pos->getPosition());

		e_collider->controller->setPosition(pos);
	}
}



void CModuleLogic::deleteEnemies()//Modificar BTs con controles para que si se llama a esta función no pete
{
	VHandles enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (auto h : enemies) {
		h.destroy();
	}
}
