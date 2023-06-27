#include "mcv_platform.h"
#include "engine.h"
#include "components/obstacle/comp_obstacle.h"
#include "modules/module_navmesh.h"



DECL_OBJ_MANAGER("comp_obstacle", TCompObstacle);

void TCompObstacle::debugInMenu() {

}

void TCompObstacle::load(const json& j, TEntityParseContext& ctx) {
	pos = loadVEC3(j, "pos");
	radius = j.value("radius",radius);
    height = j.value("height", height);
	//EngineNavmesh.addTempObstacle(pos, radius, height);
}

void TCompObstacle::update(float dt) {
	
}

void  TCompObstacle::addObstacle() {
	referenciaObstacle = EngineNavmesh.addTempObstacle(pos, radius, height);
}



void TCompObstacle::removeObstacle() {
	if (referenciaObstacle != 1){
		EngineNavmesh.removeTempObstacle(referenciaObstacle);
	}
}

void TCompObstacle::registerMsgs() {
	DECL_MSG(TCompObstacle, TMsgEntityCreated, onEntityCreated);
}

void TCompObstacle::onEntityCreated(const TMsgEntityCreated& msg) {
	referenciaObstacle = EngineNavmesh.addTempObstacle(pos, radius, height);
}
