#include "mcv_platform.h"
#include "comp_turret_bullet.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_tags.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "input/module_input.h"
#include "modules/module_physics.h"

using namespace physx;
std::mt19937 bt_mt_bu(std::random_device{}());
std::uniform_int_distribution<int> bt_mt_bu_dist(0, 10);

DECL_OBJ_MANAGER("turret_bullet_controller", TCompTurretBulletController);

void TCompTurretBulletController::debugInMenu() {
  ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 500.0f);
  ImGui::DragFloat("Damage", &intensityDamage, 0.1f, 0.0f, 100.0f);
}

void TCompTurretBulletController::load(const json& j, TEntityParseContext& ctx) {
	speed = j.value("speed", speed);
	intensityDamage = j.value("damage", intensityDamage);
	mass = j.value("mass", mass);
	fly_time = j.value("fly_time", fly_time);
}

void TCompTurretBulletController::registerMsgs() {
	DECL_MSG(TCompTurretBulletController, TMsgAssignBulletOwner, onBulletInfoMsg);
  DECL_MSG(TCompTurretBulletController, TMsgOnContact, onCollision);
}

void TCompTurretBulletController::onCollision(const TMsgOnContact& msg) {

	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	TCompCollider* c_col = source_of_impact->get<TCompCollider>();

	PxShape* colShape;
	c_col->actor->getShapes(&colShape, 1, 0);
	PxFilterData col_filter_data = colShape->getSimulationFilterData();
	if (col_filter_data.word0 & EnginePhysics.Scenario || col_filter_data.word0 & EnginePhysics.Player) {
		explode();
	}
}

void TCompTurretBulletController::explode() {
	dbg("Eldumplig explota .\n");
	//Generate damage sphere
	TCompTransform* c_trans = get<TCompTransform>();
	TMsgDamage msg;
	msg.senderType = EntityType::ENVIRONMENT;
	msg.targetType = EntityType::ALL;
	msg.position = c_trans->getPosition();
	msg.intensityDamage = intensityDamage;
	msg.impactForce = _explosionForce;
	GameController.generateDamageSphere(c_trans->getPosition(), _explosionRadius, msg, "VulnerableToExplosions");
	//Get rid of the object's collider (the shape)
	TCompCollider* c_col = get<TCompCollider>();
	static const physx::PxU32 max_shapes = 8;
	physx::PxShape* shapes[max_shapes];
	const physx::PxU32 bufferSize = 256;
	c_col->actor->getShapes(shapes, bufferSize);
	physx::PxShape* sh = shapes[0];
	//c_col->actor->detachShape(*sh); //comento para que no se vaya por debajo del suelo

	//_isFireEnabled = true;
	//TCompRender* crender = get<TCompRender>();
	//crender->showMeshesWithState(1);

	//_hasExploded = true;
	GameController.spawnPrefab("data/prefabs/vfx/boom.json", c_trans->getPosition());
    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Hazards/Explosion");
    audio.set3DAttributes(*c_trans);
	impact = true;
}

void TCompTurretBulletController::onBulletInfoMsg(const TMsgAssignBulletOwner& msg) {
	
	h_sender = msg.h_owner;
	TCompTransform* c_trans = get<TCompTransform>();
	//c_trans = get<TCompTransform>();
	//c_trans->setPosition(msg.source);
	
	
	TCompCollider* c_collider = get<TCompCollider>();
	c_collider = get<TCompCollider>();
	VEC3 new_pos = c_trans->getFront();
	PxVec3 velocidad = PxVec3(new_pos.x, new_pos.y, new_pos.z);


	launch();

}

void TCompTurretBulletController::update(float delta) {
	PROFILE_FUNCTION("TurretBullet");
	if (life_time <= 0 || impact) {
		CHandle(this).getOwner().destroy();
	}
	else {
		life_time -= delta;
	}
}


VEC3 TCompTurretBulletController::getForceFrom(VEC3 fromPos, VEC3 toPos)
{
	return ( VEC3(toPos.x, toPos.y, toPos.z) -  VEC3(fromPos.x, fromPos.y,fromPos.z));
}


void TCompTurretBulletController::launch() {
	
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
	VEC3 projectileXZPos = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);//porque sale del suelo


	VEC3 forceDirection = getForceFrom(projectileXZPos, targetXZPos);

	x = forceDirection.x;         
	y = forceDirection.y;         
	z = forceDirection.z;        

	
	x0 = 0.;
	y0 = 0.;
	z0 = 0.;

	
	v0x = (x - x0) / fly_time;
	v0z = (z - z0) / fly_time;
	v0y = (y - y0 + (0.5f * 9.8f * pow(fly_time, 2))) / fly_time;



	VEC3 vec3r = VEC3(1, 0, 0) * v0x;
	VEC3 vec3u = VEC3(0, 1, 0) * v0y;
	VEC3 vec3f = VEC3(0, 0, 1)  * v0z;
	rigid_dynamic->addForce(VEC3_TO_PXVEC3(vec3r), PxForceMode::eVELOCITY_CHANGE);
	rigid_dynamic->addForce(VEC3_TO_PXVEC3(vec3u), PxForceMode::eVELOCITY_CHANGE);
	rigid_dynamic->addForce(VEC3_TO_PXVEC3(vec3f), PxForceMode::eVELOCITY_CHANGE);

	float x_angular, y_angular, z_angular;
	//int dice = bt_mt_bu_dist(bt_mt_bu);
	x_angular = bt_mt_bu_dist(bt_mt_bu);
	y_angular = bt_mt_bu_dist(bt_mt_bu);
	z_angular = bt_mt_bu_dist(bt_mt_bu);
	

	rigid_dynamic->setAngularVelocity(VEC3_TO_PXVEC3(VEC3(x_angular, y_angular, z_angular)));

}






