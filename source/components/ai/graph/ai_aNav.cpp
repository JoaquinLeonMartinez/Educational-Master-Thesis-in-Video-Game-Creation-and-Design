#include "mcv_platform.h"
#include "ai_anav.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_render.h"
#include "components/common/comp_tags.h"
//To be removed/changed, needed for identification
#include "components/common/comp_name.h"

#include "components/john/comp_bullet.h"
#include "modules/module_physics.h"

#include "PxPhysicsAPI.h"


#include "time.h"
#include "stdlib.h"
#include "random"


DECL_OBJ_MANAGER("ai_anav", CAIAnav);
using namespace physx;

std::mt19937 mt(1729);
std::uniform_int_distribution<int> dist(0, 100);

/* --------------------- Helper Functions ---------------------*/
/*DEPRECATED*/
//void CAISushi::LookAt_Player(TCompTransform* c_trans, TCompTransform* player_position, float dt) {
//	//Rotation Control
//	float yaw, pitch;
//	c_trans->getAngles(&yaw, &pitch);
//	if (c_trans->isInLeft(player_position->getPosition())) {
//		c_trans->setAngles(yaw + dt + deg2rad(rotationSpeed), pitch);
//	}
//	else {
//		c_trans->setAngles(yaw - dt - deg2rad(rotationSpeed), pitch);
//	}
//	//End Rotation Control
//}




bool CAIAnav::isGrounded() {
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 player_pos = c_trans->getPosition();
	player_pos.y += 0.0001f; //So we can detect collision with planes
	PxVec3 origin = VEC3_TO_PXVEC3(player_pos);
	PxVec3 unitDir = VEC3_TO_PXVEC3(VEC3(0, -1, 0));
	PxReal maxDistance = 1.0f;
	PxRaycastBuffer hit;



	bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit);
	if (hit.hasBlock) {
		PxF32 dist = hit.block.distance;
		if (dist <= 0.2f) {
			return true;
		}
	}
	else if (hit.hasAnyHits()) {
		PxF32 dist = hit.touches->distance;
		if (dist <= 0.2f) {
			return true;
		}
	}
	return false;
}



void CAIAnav::generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc)
{
	navmeshPath = EngineNavmesh.findPath(initPos, destPos);
	navmeshPathPoint = 0;
	recalculateNavmesh = recalc;
}


/* -------------------- End Helper Functions --------------------*/

void CAIAnav::Init()
{
	

	AddState("SEEKWPT", (statehandler)&CAIAnav::SeekwptState);
	AddState("NEXTWPT", (statehandler)&CAIAnav::NextwptState);
	
	//---------MAPA RESTAURANTE-------------------------------------------------------------------------------
    //patrolPoints.push_back(VEC3(-16.f, 0.f, 54.f)); //pasar por ostaculo en el lado que no hay estanterias
	//patrolPoints.push_back(VEC3(-16.f, 0.f, 48.f));//prueba en medio de obstaculo;
	//patrolPoints.push_back(VEC3(4.0f, 0.f, 28.0f));//hacia el capullo antes de tocar valores de pathfind pero ahora llega
	//patrolPoints.push_back(VEC3(24.f, 0.f, -11.f));//caminando por la zona de muchas tiles
	//patrolPoints.push_back(VEC3(-4.f, 0.f, -23.f)); //pasar por estructura de mapa
	//patrolPoints.push_back(VEC3(-23.f, 0.f, -44.f));//zona de navmesh liosa en el este
	//patrolPoints.push_back(VEC3(0.f, 0.f, -13.f)); //zona de navmesh liosa en el centro
	//patrolPoints.push_back(VEC3(-16.f, 0.f, -50.f));//punto para evitar dos obstaculos en la zona navmesh liosa. PRUEBA BUENA DEL COPON
	//patrolPoints.push_back(VEC3(-5.f, 0.f, 40.f));
	//patrolPoints.push_back(VEC3(-30.f, 0.f, 32.f));
	//-----------------------------------------------------------------------------------------------------------

	//MAPA SUPERMARKET MILESTONE 2 PRUEBA
	patrolPoints.push_back(VEC3(-56.f, 0.f, -81.f));
	
	

	ChangeState("SEEKWPT");
}


void CAIAnav::NextwptState(float dt) {
	last_state = state;
	TCompTransform* c_trans = get<TCompTransform>();
	if (wtpIndex < navmeshPath.size() - 1) {
		wtpIndex = (wtpIndex + 1) % navmeshPath.size();
		nextPoint = navmeshPath[wtpIndex];
		dbg("Next Waypoint is %f, %f, %f\n", nextPoint.x, nextPoint.y, nextPoint.z);
		ChangeState("SEEKWPT");
	}
	else {
		//FIN
	}
	
	/*
	TCompRender* render = get<TCompRender>();
	render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0);
	*/
}

void CAIAnav::SeekwptState(float dt) {
	last_state = state;
	if (!h_player.isValid()) {
		h_player = getEntityByName("Player");
		return;
	}

	if (init) {
		init = false;
		TCompTransform* c_trans = get<TCompTransform>();
		VEC3 position = c_trans->getPosition();
		//wtp 
		generateNavmesh(position, patrolPoints[0], false);
		
		if (navmeshPath.size() <= 0){
			return;
		}
		
	nextPoint = navmeshPath[wtpIndex];
	}

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	currentPosition = c_trans->getPosition();

	//Gravity control
	Vector3 dir = VEC3(0.f, gravity, 0.f);
	dir = c_trans->getFront() * patrolSpeed;
	dir *= dt;
	//MOVE PLAYER
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	VEC3 m_hitPos = VEC3();
	
	/*
	dtTileCache* m_tileCache = EngineNavmesh.getNavmesh()->getNav().m_tileCache;
	if (EngineNavmesh.wallDistance(currentPosition) < 1.5f) {
		dbg("Choco contra algo");
		generateNavmesh(c_trans->getPosition(), patrolPoints[0], false);
		wtpIndex = 0;
		nextPoint = navmeshPath[wtpIndex];
	}*/
	/*
	VEC3 frontOffset = VEC3(currentPosition.x, currentPosition.y, currentPosition.z + 0.5);
	bool memeto = EngineNavmesh.raycast(currentPosition, frontOffset,m_hitPos);
	if (memeto) {
		VEC3 position = c_trans->getPosition();
		//wtp 
		dbg("Choco contra algo");
		generateNavmesh(position, patrolPoints[0], false);
		wtpIndex = 0;
		nextPoint = navmeshPath[wtpIndex];
	}
	*/
	//if(EngineNavmesh.raycast(currentPosition,))


	if (Vector3::Distance(nextPoint, c_trans->getPosition()) < distanceCheckThreshold) {

		ChangeState("NEXTWPT");
	}
	else {
		float yaw, pitch;
		c_trans->getAngles(&yaw, &pitch);
		if (c_trans->isInLeft(nextPoint)) {
			c_trans->setAngles(yaw + dt, pitch);
		}
		else {
			c_trans->setAngles(yaw - dt, pitch);
		}
	}
	
}



void CAIAnav::load(const json& j, TEntityParseContext& ctx) {
	this->Init();

}

void CAIAnav::debugInMenu() {
	
}

void CAIAnav::renderDebug() {
	
}







