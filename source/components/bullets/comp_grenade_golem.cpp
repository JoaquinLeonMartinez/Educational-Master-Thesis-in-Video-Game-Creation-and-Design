#include "mcv_platform.h"
#include "comp_grenade_golem.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_tags.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "input/module_input.h"
#include "modules/module_physics.h"
#include "components/common/comp_name.h"

#include "PxPhysicsAPI.h"

using namespace physx;

DECL_OBJ_MANAGER("grenade_golem_controller", TCompGrenadeGolemController);

void TCompGrenadeGolemController::debugInMenu() {
	

	
}


void TCompGrenadeGolemController::load(const json& j, TEntityParseContext& ctx) {
	//init_time_explosion = j.value("speed_explosion", init_time_explosion);
	explosion_radius = j.value("explosion_radius", explosion_radius);
}

void TCompGrenadeGolemController::registerMsgs() {
	DECL_MSG(TCompGrenadeGolemController, TMsgAssignBulletOwner, onGrenadeInfoMsg);
	DECL_MSG(TCompGrenadeGolemController, TMsgOnContact, onCollision);
}

void TCompGrenadeGolemController::onCollision(const TMsgOnContact& msg) {
  CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
  TCompCollider* c_col = source_of_impact->get<TCompCollider>();

  dbg("el proyectil cupcake detecta la colision con el suelo - - - - - - - - - - - - - - - -\n");

  PxShape* colShape;
  c_col->actor->getShapes(&colShape, 1, 0);
  PxFilterData col_filter_data = colShape->getSimulationFilterData();
  if (col_filter_data.word0 & EnginePhysics.All) { //EnginePhysics.Scenario
    flagExplota = true;
	TCompTransform* c_trans = get<TCompTransform>();
	
	std::string _prefab = "data/prefabs/enemies/bt_cupcake.json";
	
	enemy = GameController.spawnPrefab(_prefab, c_trans->getPosition());
		
	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();

  }
}


void TCompGrenadeGolemController::launch() {

	float x;
	float y;
	float z;
	float x0;
	float y0;
	float z0;
	float v0x;
	float v0y;
	float v0z;

	TCompCollider* c_collider = get<TCompCollider>();
	c_collider = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);


	CHandle h_player = GameController.getPlayerHandle();
	CEntity* e_player = (CEntity*)h_player;
	TCompTransform* player_trans = e_player->get< TCompTransform>();
	VEC3 targetXZPos = VEC3(player_trans->getPosition().x, player_trans->getPosition().y, player_trans->getPosition().z);
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 projectileXZPos = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);


	VEC3 forceDirection = getForceFrom(projectileXZPos, targetXZPos);

	x = forceDirection.x;
	y = forceDirection.y;
	z = forceDirection.z;


	x0 = 0.;
	y0 = c_trans->getPosition().y;
	z0 = 0.;


	v0x = (x - x0) / fly_time;
	v0z = (z - z0) / fly_time;
	v0y = (y - y0 + (0.5f * 9.8f * pow(fly_time, 2))) / fly_time;



	VEC3 vec3r = VEC3(1, 0, 0) * v0x;
	VEC3 vec3u = VEC3(0, 1, 0) * v0y;
	VEC3 vec3f = VEC3(0, 0, 1)  * v0z;
	rigid_dynamic->addForce(VEC3_TO_PXVEC3(vec3r), PxForceMode::eIMPULSE);
	rigid_dynamic->addForce(VEC3_TO_PXVEC3(vec3u), PxForceMode::eIMPULSE);
	rigid_dynamic->addForce(VEC3_TO_PXVEC3(vec3f), PxForceMode::eIMPULSE);
}

VEC3 TCompGrenadeGolemController::getForceFrom(VEC3 fromPos, VEC3 toPos)
{
	return (VEC3(toPos.x, toPos.y, toPos.z) - VEC3(fromPos.x, fromPos.y, fromPos.z));
}

void TCompGrenadeGolemController::onGrenadeInfoMsg(const TMsgAssignBulletOwner& msg) {
	h_sender = msg.h_owner;
	TCompTransform* c_trans = get<TCompTransform>();
	c_trans->setPosition(msg.source);
	float yaw = vectorToYaw(msg.front);
	c_trans->setAngles(yaw, 0.f, 0.f);

	dbg("Naces en x:%f,y:%f,z:%f\n", c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z + 1.f);
	
	c_collider = get<TCompCollider>();
	
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);

	PxVec3 pxvec3 = PxVec3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
	PxTransform pxtransf = PxTransform(pxvec3);
	rigid_dynamic->setGlobalPose(pxtransf);
	launch();
	
}

void TCompGrenadeGolemController::update(float delta) {//cambiar la condicion un poco antes de la explosion para indicar que va a explotar
	PROFILE_FUNCTION("GrenadeGolem");

	if (flagExplota) {
		//Le decimos al cupcake quien es su padre
	
		TMsgSpawnerCheckin checkin;
		checkin.spawnerHandle = h_sender;
		((CEntity*)enemy)->sendMsg(checkin);

		//Le decimos al golem quien es su hijo
		TMsgSpawnerFather msg;
		msg.son = enemy;
		((CEntity*)h_sender)->sendMsg(msg);
		flagExplota = false;
		

	}

}

	

