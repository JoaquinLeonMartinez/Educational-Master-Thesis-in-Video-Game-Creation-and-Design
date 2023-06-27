#include "mcv_platform.h"

#include "module_game_controller.h"

#include "components/controllers/comp_sCart_controller.h"
#include "components/objects/checkpoints/checkpoint.h"
#include "components/common/comp_tags.h"
#include "input/module_input.h"
#include "components/ai/bt/bt_sushi.h"
#include "modules/gameplay_fragments/module_ambush.h"
#include "components/ai/bt/bt_ranged_sushi.h"
#include "components/ai/bt/bt_golem.h"
#include "components/controllers/comp_trans_curve_controller.h"
#include "modules/module_camera_mixer.h"
#include "components/controllers/camera/comp_camera_3rd_person.h"
#include "components/ai/graph/ai_platform.h"
#include "components/common/comp_name.h"
#include "components/postfx/comp_render_radial_blur.h"
#include "entity/entity.h"
#include "modules/module_scenes.h"
#include "components/ai/bt/bt_cupcake.h"
#include "components/ai/others/comp_blackboard.h"
#include "components/objects/comp_wind_trap.h"
#include "components/actions/comp_audioPlayer.h"
#include "ui/module_ui.h"
#include "render/module_render.h"
#include "components/postfx/comp_render_bloom.h"



bool CModuleGameController::start() {

   /* window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_width = 250;
    window_height = 150;

    _player = EngineEntities.getPlayerHandle();

    _lastCheckpoint = new CCheckpoint();
    _currentstate = PauseState::none;
    menuPosition = 0;
    */
    _lastCheckpoint = new CCheckpoint();
    return true;
}

void CModuleGameController::wakeUpWinds() {
  CEntity* e1 = getEntityByName("Trap_air_001");
  TCompWindTrap* cw_1 = e1->get<TCompWindTrap>();
  cw_1->enableWind();
  CEntity* e2 = getEntityByName("Trap_air_002");
  TCompWindTrap* cw_2 = e2->get<TCompWindTrap>();
  cw_2->enableWind();
}

void CModuleGameController::setGodMode(bool _god_mode) {
	god_mode = _god_mode; 
}


void CModuleGameController::setResurrect(bool _resurrect) {
	resurrect = _resurrect;
}


void CModuleGameController::update(float delta) {

    //updateGameCondition();
  if (EngineInput["godMode_"].justPressed()) {
    god_mode = !god_mode;
  }
  else if (EngineInput["flyMode_"].justPressed()) {
    CEntity* e_player = getEntityByName("Player");
  }
  else if (EngineInput["block_invisible_cursor_"].justPressed()) {
	  invisible_block_cursor = !invisible_block_cursor;
  }

}

void CModuleGameController::stop()
{
    if (_lastCheckpoint) {
        delete _lastCheckpoint;
		_lastCheckpoint = nullptr;
    }
}

CHandle CModuleGameController::getPlayerHandle() {
	return EngineEntities.getPlayerHandle();
}

#pragma region Checkpoints
bool CModuleGameController::isCheckpointSaved() {
	return _lastCheckpoint != nullptr && _lastCheckpoint->isSaved(); 
}
/*

This function calls CCheckpoint::saveCheckPoint with the player position and rotation
This updates _lastCheckpoint's data with the status of those entities marked to be registered

*/
bool CModuleGameController::saveCheckpoint(VEC3 playerPos, QUAT playerRot) {

    return _lastCheckpoint->saveCheckPoint(playerPos, playerRot);
}

bool CModuleGameController::loadCheckpoint() {

    if (_lastCheckpoint) {
        return _lastCheckpoint->loadCheckPoint();
    }
    else {
        return false;
    }
}

bool CModuleGameController::deleteCheckpoint() {

    if (_lastCheckpoint) {
        return _lastCheckpoint->deleteCheckPoint();
    }
    else {
        return false;
    }
}
#pragma endregion

/*CModuleGameController::PauseState CModuleGameController::getCurrentState() {

    return _currentstate;
}*/

PowerType CModuleGameController::getPowerSelected() {
	if (_lastCheckpoint != nullptr) {
		return _lastCheckpoint->getPlayerStatus().power_selected;
	}
	else {
		return PowerType::TELEPORT;
	}
}



void CModuleGameController::savePower(PowerType power) {
	_lastCheckpoint->savePower(power);
}

#pragma region Behavior Tree Control
void CModuleGameController::stopStateMachines() {
    VHandles handles_in_scene = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
    for (auto& instance : handles_in_scene) {
        TMsgAIPaused msgPaused;
        msgPaused.isPaused = true;
        instance.sendMsg(msgPaused);
    }
}

void CModuleGameController::stopBehaviorTrees() {
	VHandles handles_in_scene = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (auto& instance : handles_in_scene) {
		TMsgBTPaused msgPaused;
		msgPaused.isPaused = true;
		instance.sendMsg(msgPaused);
	}	
}

void CModuleGameController::resumeBehaviorTrees() {
	VHandles handles_in_scene = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (auto& instance : handles_in_scene) {
		TMsgBTPaused msgPaused;
		msgPaused.isPaused = false;
		instance.sendMsg(msgPaused);
	}
}
#pragma endregion

#pragma region Entity Handling
//Spawn a prefab and get it's CHandle
CHandle CModuleGameController::spawnPrefab(const std::string prefab, Vector3 pos, QUAT rot, float scale) {
	TEntityParseContext ctx;
  ctx.root_transform.setPosition(pos);
  ctx.root_transform.setRotation(rot);
  ctx.root_transform.setScale(scale);
	bool result = parseScene(prefab, ctx);
	assert(result);
	return ctx.entities_loaded[0];
}
//
void CModuleGameController::toNamePrefab(const std::string name, CHandle handle) {
	CEntity* e = handle;
	TCompName* c_name = e->get<TCompName>();
}



//lo que se quiere es spawnear algo en la posicion de otra entidad dado el nombre
//ej:spawnSushy("Dummy1")
void CModuleGameController::spawnPrefabByPoint(const std::string prefab, std::string origin) {
	CEntity* entity = (CEntity *) getEntityByName(origin);
	TCompTransform* transform = entity->get<TCompTransform>();
	spawnPrefab(prefab, transform->getPosition());
}

//activar aniamcion de morph
void CModuleGameController::playAnimationMorph(std::string name) {
	CEntity* entity = (CEntity *)getEntityByName(name);
	TCompMorphAnimation* morphAnimation = entity->get<TCompMorphAnimation>();
	morphAnimation->play_ = true;
	
}
//desactivar animacion de morph
void CModuleGameController::stopAnimationMorph(std::string name) {
	CEntity* entity = (CEntity *)getEntityByName(name);
	TCompMorphAnimation* morphAnimation = entity->get<TCompMorphAnimation>();
	morphAnimation->play_ = false;
}



//danyo a una entidad por el nombre
void CModuleGameController::setDamage(const std::string name_origin, int damage) {
	CEntity* entity = (CEntity *)getEntityByName(name_origin);
	TMsgDamageToAll message;
	message.intensityDamage = damage;
	entity->sendMsg(message);
}


//bindear componente curva
void CModuleGameController::bindInCurve(std::string _curve, std::string name_origin) {

	CHandle  h_entity = getEntityByName(name_origin);
	CEntity* entity = h_entity;
	CEntity* entity_curve = getEntityByName(_curve);
	CHandle h_curve  = entity_curve->get<TCompTransCurveController>();
	entity->set(h_curve.getType(), h_curve);
	
	dbg("assignando handle %08x to emtity %08x\n", h_curve.asUnsigned(), h_entity.asUnsigned());
	dbg("Componente padre %08x \n", h_curve.getOwner());
	
	//CHandle enhandletity = getEntityByName(name_origin);
	//enhandletity.setOwner(tcompCurve);
	//entity->set(handle.getType(), handle);
}
//blendear camera
void CModuleGameController::blendingCamera(std::string camDest, float blendTime, std::string typeInterpolation) {
	CHandle  h_entity = getEntityByName(camDest);
	if(h_entity.isValid()){
		Engine.getCameraMixer().blendCamera(h_entity, blendTime, Engine.getCameraMixer().getInterpolator(typeInterpolation));
	}
}

//bloquearCamera Player
void CModuleGameController::lockCamera3Person(bool activate) {
	CEntity* e_cam3 = getEntityByName("PlayerCamera");
	TCompCamera3rdPerson* t_comp3 = e_cam3->get<TCompCamera3rdPerson>();
	if (t_comp3 != nullptr) {
		//t_comp3->_enabled = activate;
		t_comp3->mouse_active = activate;
		t_comp3->_enabled = activate;
	
	}
}

void CModuleGameController::blendPlayerCamera() {
  static Interpolator::TQuadInOutInterpolator quadInt;
  CEntity* p_camera = getEntityByName("PlayerCamera");
  Engine.getCameraMixer().blendCamera(p_camera, 0.3f, &quadInt);

  CEntity* m_camera = getEntityByName("MainCamera");
  TCompRenderRadialBlur* c_rrb = m_camera->get<TCompRenderRadialBlur>();
  c_rrb->enable(0.3f);
}

//resetCamera
void CModuleGameController::resetCamera() {
	CEntity* e_cam3 = getEntityByName("PlayerCamera");
	if(e_cam3 != nullptr){
		TCompCamera3rdPerson* t_comp3 = e_cam3->get<TCompCamera3rdPerson>();
		if (t_comp3 != nullptr) {
			//t_comp3->_enabled = activate;
			t_comp3->resetCamera();
		}
	}
}

void CModuleGameController::dbgInLua(std::string text) {
	dbg("EN LUA:%s\n",text.c_str());
}

void CModuleGameController::destroyCHandleByName(std::string name) {
	if (name != "") {
		CHandle h = getEntityByName(name);
		if (h.isValid()) {
			h.destroy();
		}
	}
}


void CModuleGameController::deleteCupcake()
{
	VHandles enemies = CTagsManager::get().getAllEntitiesByTag(getID("cupcake"));
	for (auto h : enemies) {

		TMsgDeleteTrigger msg;
		msg.deleteForTrigger = true;
		msg.h_entity = h;
		msg.damage = 999.f;

		CEntity* e_enemy = (CEntity*)h;
		e_enemy->sendMsg(msg);
	}
}
void CModuleGameController::deleteSushi()
{
	VHandles enemies = CTagsManager::get().getAllEntitiesByTag(getID("sushi"));
	for (auto h : enemies) {
		
		TMsgDeleteTrigger msg;
		msg.deleteForTrigger = true;
		msg.h_entity = h;
		msg.damage = 100.f;
		
		CEntity* e_enemy = (CEntity*)h;
		e_enemy->sendMsg(msg);
	}
} 
void CModuleGameController::deleteGolem(std::string name)
{
	
	if(name != ""){
		CEntity* e_golem = getEntityByName(name);
		if (e_golem != nullptr){
			TCompSelfDestroy* comp = e_golem->get<TCompSelfDestroy>();
			comp->setEnabled(true);
		}
	}
	
}

void CModuleGameController::setLifeEnemiesByTag(const char* tagName,float life) {
	VHandles enemies = CTagsManager::get().getAllEntitiesByTag(getID(tagName));
	for (auto h : enemies) {
		CEntity* e_enemy = (CEntity*)h;

		if (strcmp(tagName,"cupcake") == 0) {
			CBTCupcake* cbt = e_enemy->get<CBTCupcake>();
			cbt->setLife(life);
		}
		else if (strcmp(tagName, "golem") == 0) {


		}
		else if (strcmp(tagName, "sushi") == 0) {

		}
	}
	/*
	if (typeEnemy == TYPE_CUPCAKE) {
		CBTCupcake* cupcake = e_enemy->get<CBTCupcake>();
		cupcake->setLife(life);
	}
	if (typeEnemy == TYPE_SUSHI) {
		CBTSushi* sushi_n = e_enemy->get<CBTSushi>();
		//sushi_n->setLifesetLife no esta el set en la clase
	}
	if (typeEnemy == TYPE_RANGED_SUSHI) {
		CBTRangedSushi* sushi_r = e_enemy->get<CBTRangedSushi>();
		//sushi_n->setLifesetLife: no esta el set en la clase
	}*/

}



void CModuleGameController::activatePlatformByName(std::string name) {
	if (name != "") {
		CEntity* e_platform = getEntityByName(name);
		CAIMobilePlatform* c_platform = e_platform->get<CAIMobilePlatform>()  ;
		c_platform->active = true;
	}
}

//Spawn a puddle and get it's CHandle
CHandle CModuleGameController::spawnPuddle(Vector3 pos, QUAT rot, float scale) {
    VEC3 desiredPosition = pos;
    desiredPosition.y += 0.001f; //So we can detect collision with planes
    physx::PxRaycastHit hit;
    physx::PxFilterData pxFilterData;
    pxFilterData.word0 = EnginePhysics.Scenario;
    physx::PxQueryFilterData PxScenarioFilterData;
    PxScenarioFilterData.data = pxFilterData;
    PxScenarioFilterData.flags = physx::PxQueryFlag::eSTATIC;
    EnginePhysics.Raycast(desiredPosition, VEC3(0, -1, 0), 30.f, hit, physx::PxQueryFlag::eSTATIC, PxScenarioFilterData);
    if (hit.position != physx::PxVec3(0)) {
        desiredPosition = PXVEC3_TO_VEC3(hit.position);
    }

    return spawnPrefab("data/prefabs/structures/madness_puddle.json", desiredPosition, rot, scale);;
}

void CModuleGameController::deleteByTag(const char* tag) {
	VHandles handles_in_scene = CTagsManager::get().getAllEntitiesByTag(getID(tag));
	for (auto& instance : handles_in_scene) {
		auto hm = CHandleManager::getByType(instance.getType());
		if (hm) {
			hm->destroyHandle(instance);
		instance.getOwner().destroy();
		instance.destroy();
		}
	}
}
#pragma endregion


#pragma region Time Control
void CModuleGameController::stopTime() {
	setTimeScale(0.0f);
}

void CModuleGameController::resumeTime() {
	setTimeScale(1.0f);
}

void CModuleGameController::setTimeScale(float timeScale){
	assert(timeScale >= 0.0f);
	Time.scale_factor = timeScale;
}

float CModuleGameController::getTimeScale(){
	return Time.scale_factor;
}
#pragma endregion

void CModuleGameController::addMadnessPuddle() {
	_madnessPuddlesCleansed++;
}

int CModuleGameController::getMadnessPuddles() {
	return _madnessPuddlesCleansed;
}

void CModuleGameController::addEnemiesKilled(EntityType e) {
  if (gf)
    gf->enemyKilled(e);
	enemiesKilled++;
}

int CModuleGameController::getEnemiesKilled() {
	return enemiesKilled;
}

float CModuleGameController::getHpBarSize() {
	return hp_bar_size;
}

void CModuleGameController::increaseHpBarSize(float _hp_bar_size) {
	hp_bar_size += _hp_bar_size;
}

void CModuleGameController::increaseMadnessBarSize(float _madness_bar_size) {
	madness_bar_size += _madness_bar_size;
}

float CModuleGameController::getMadnessBarSize() {
	return madness_bar_size;
}




void CModuleGameController::generateDamageSphere(VEC3 center, float radius, TMsgDamage message, const char* targetFilter) {
	//Create a geometry
	physx::PxSphereGeometry geometry(radius);
	//Create filter data
	physx::PxFilterData pxFilterData;
	pxFilterData.word0 = EnginePhysics.getFilterByName(targetFilter);
	//Create query filter data
	physx::PxQueryFilterData queryFilterData;
	queryFilterData.data = pxFilterData;
	queryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
	//Create vector to store the hits
	std::vector<physx::PxOverlapHit> hits;
	//Overlap and check for collissions
	if (EnginePhysics.Overlap(geometry, center, hits, queryFilterData)) {
		for (int i = 0; i < hits.size(); i++) {
			CHandle hitCollider;
			hitCollider.fromVoidPtr(hits[i].actor->userData);
			if (hitCollider.isValid()) {
				//Send the message we've been given
				CEntity* entityContact = hitCollider.getOwner();
				entityContact->sendMsg(message);
				//Check if we can apply force to the entity
				/*TCompRigidBody* c_rb = entityContact->get<TCompRigidBody>();
				if (c_rb && impactForce != 0.f) {
					TCompTransform* entityTransform = entityContact->get<TCompTransform>();
					VEC3 resultingForce = entityTransform->getPosition() - center;
					resultingForce.Normalize();
					resultingForce *= impactForce;
					c_rb->addForce(resultingForce);
				}*/
			}
		}
	}
}

void CModuleGameController::healPlayer() {
	CEntity* e_player = (CEntity *)EngineEntities.getPlayerHandle();
	TCompCharacterController* character_controller = e_player->get<TCompCharacterController>();
	character_controller->heal();
}

void CModuleGameController::destroyWallByName(std::string name, std::string golemName, float intensity) {

	
	//obtener golem para que el muro se destruya como debe
	try {

	}
	catch (exception e) {};
	CEntity* e_golem = getEntityByName(golemName);

	if (e_golem != nullptr) {
		TMsgActiveGolem msg;
		msg.active = true;
		e_golem->sendMsg(msg);
	}

	//-------------


	CEntity* e_wall = getEntityByName(name);

	TCompTransform* wall_trans = e_wall->get<TCompTransform>();
	VEC3 wall_pos = wall_trans->getPosition();

	if (e_wall != nullptr) {
		TMsgDamage msg;//quiza seria mejor utilizar otro tipo de mensaje si queremos diferenciar entre cuando lo destruye el player o la magia de LUA
		msg.damageType = MELEE;
		msg.impactForce = intensity;
		msg.h_sender = e_golem;

		e_wall->sendMsg(msg);
	}


}

void CModuleGameController::wakeUpGolem(std::string name) {

	CEntity* e_golem = getEntityByName(name);

	if (e_golem != nullptr) {
		TMsgActiveGolem msg;
		msg.active = true;
		e_golem->sendMsg(msg);
	}
}

void CModuleGameController::sleepGolem(std::string name) {

	CEntity* e_golem = getEntityByName(name);

	if (e_golem != nullptr) {
		TMsgActiveGolem msg;
		msg.active = false;
		e_golem->sendMsg(msg);
	}

}

void CModuleGameController::healPlayerPartially(float health) {
    CEntity* e_player = (CEntity *)EngineEntities.getPlayerHandle();
    TCompCharacterController* character_controller = e_player->get<TCompCharacterController>();
    character_controller->healPartially(health);
}

void CModuleGameController::restoreMadness() {
	CEntity* e_player = (CEntity *)EngineEntities.getPlayerHandle();
	TCompCharacterController* character_controller = e_player->get<TCompCharacterController>();
	character_controller->restoreMadness();
}

void CModuleGameController::addPowerUp(float quantity, PowerUpType type) {

	//de momento el isLife seria el tipo y solo diferenciamos entre si es life o es energia/locura
	//en un futuro lo cambiamos por un type para usarlo en los demas powerUps
	//dbg("el game controller envia el power up de life  \n");
	CEntity* e_player = (CEntity *) EngineEntities.getPlayerHandle();
	TCompCharacterController* character_controller = e_player->get<TCompCharacterController>();
	character_controller->applyPowerUp(quantity,type, extraBarSize);

}

void CModuleGameController::renderInMenu() {
  if (ImGui::TreeNode("Game Controller"))
  {
    if (gf)
      gf->renderInMenu();
    ImGui::TreePop();
  }
}

void CModuleGameController::activateGameplayFragment(string module) {
  //fatal(module.c_str());
  if (gf) { //JUST IN CASE
    gf->stop();
  }

  gf = new CModuleAmbush(1); //SHIT HERE
  gf->start();
}



void CModuleGameController::deactivateGameplayFragment(string module) {
  //fatal(module.c_str());
  if (gf) {
    gf->stop();
  }
}

void CModuleGameController::activateSecondEvent() {
	//fatal(module.c_str());
	if (gf) {
		gf->secondEvent();
	}
}

int CModuleGameController::getNumThermoStatesActives() {
	if(gf != nullptr){
		return gf->getThermostatsActivates();
	}
	return 0;
}

void CModuleGameController::wakeUpSushi(string id) {
	
	CHandle sushi = getEntityByName("Sushi_"+id);
	if (sushi.isValid()) {
		TMsgBTPaused msgPaused;
		msgPaused.isPaused = false;
		sushi.sendMsg(msgPaused);
	}
}

void CModuleGameController::deleteElement(string id) {

	CHandle element = getEntityByName(id);

	if (element.isValid()) {
		element.destroy();
	}

}

void CModuleGameController::activateSpawner(string id) {

	CHandle spawner = getEntityByName(id);
	if (spawner.isValid()) {
		TMsgSetActive msgPaused;
		msgPaused.active = true;
		spawner.sendMsg(msgPaused);
	}
}

/*Nuevo metodo de seteo de curvas para plataformas*/
void CModuleGameController::updatePlatformCurveByName(std::string _curve, std::string name_platform) {
	CHandle handle = getEntityByName(name_platform);
	if (handle.isValid()) {
		std::string path = "data/curves/" + _curve + ".curve";
		CEntity* entity = (CEntity*)handle;
		CAIMobilePlatform* platform = entity->get<CAIMobilePlatform>();
		platform->setCurve(Resources.get(path)->as<CCurve>());
	}
}



/*
_curve: nombre de la curva

enemy = nombre de la entidad

*/
void CModuleGameController::updateEnemyCurveByName(std::string _curve, std::string enemy) {

	CHandle h_enemy = getEntityByName(enemy);
	if (h_enemy.isValid()) {
		std::string path = "data/curves/" + _curve + ".curve";
		//TENER EN CUENTA QUE SI ES UN RANGED O NO
		CEntity* e_enemy = (CEntity*)h_enemy;


		CBTSushi* sushi = e_enemy->get<CBTSushi>();
		if (sushi != nullptr) {
			sushi->setCurve(Resources.get(path)->as<CCurve>());
		}
		else {
			CBTRangedSushi* sushi = e_enemy->get<CBTRangedSushi>();
			sushi->setCurve(Resources.get(path)->as<CCurve>());
		}
	}
}

//modificar nombre de metodo a suishi
void  CModuleGameController::updateEnemyCurveByHandle(std::string _curve, CHandle h_enemy) {
	if (h_enemy.isValid()) {
		std::string path = "data/curves/" + _curve + ".curve";
		//TENER EN CUENTA QUE SI ES UN RANGED O NO
		CEntity* e_enemy = (CEntity*)h_enemy;


		CBTSushi* sushi = e_enemy->get<CBTSushi>();
		if (sushi != nullptr) {
			sushi->setCurve(Resources.get(path)->as<CCurve>());
		}
		else {
			CBTRangedSushi* sushi = e_enemy->get<CBTRangedSushi>();
			sushi->setCurve(Resources.get(path)->as<CCurve>());
		}
	}
}


void  CModuleGameController::updateCupcakeCurveByHandle(std::string _curve, CHandle h_enemy) {
	if (h_enemy.isValid()) {
		std::string path = "data/curves/" + _curve + ".curve";
		//TENER EN CUENTA QUE SI ES UN RANdGED O NO
		CEntity* e_enemy = (CEntity*)h_enemy;


		CBTCupcake* cupcake = e_enemy->get<CBTCupcake>();
		if (cupcake != nullptr) {
			cupcake->setCurve(Resources.get(path)->as<CCurve>());
		}
		
	}
}

//de momento sin esclarecer el tipo de enemigo(Es un sushy ahora)
void CModuleGameController::setHeightEnemyByHandle(int height, CHandle h_enemy,int typeEnemy) {
	if (h_enemy.isValid()) {
		if(TYPE_SUSHI == typeEnemy){
			CEntity* e_enemy = (CEntity*)h_enemy;
			CBTSushi* sushi = e_enemy->get<CBTSushi>();
			sushi->setHeightRange(height);
		}
		else if (TYPE_GOLEM == typeEnemy) {
			CEntity* e_enemy = (CEntity*)h_enemy;
			CBTGolem* golem = e_enemy->get<CBTGolem>();
			golem->setHeightRange(height);
		}
    else if (TYPE_RANGED_SUSHI == typeEnemy) {
      CEntity* e_enemy = (CEntity*)h_enemy;
      CBTRangedSushi* r_sushi = e_enemy->get<CBTRangedSushi>();
      r_sushi->setHeightRange(height);
    }
		
	}
}

void CModuleGameController::setViewDistanceEnemyByHandle(float distance, CHandle h_enemy, int typeEnemy) {
	if (h_enemy.isValid()) {
		if (TYPE_GOLEM == typeEnemy) {
			CEntity* e_enemy = (CEntity*)h_enemy;
			CBTGolem* golem = e_enemy->get<CBTGolem>();
			golem->setViewDistance(70.f);
		}
		else if (TYPE_SUSHI == typeEnemy) {
			CEntity* e_enemy = (CEntity*)h_enemy;
			CBTSushi* sushi = e_enemy->get<CBTSushi>();
			sushi->setViewDistance(distance);
		}
		else if (TYPE_RANGED_SUSHI == typeEnemy) {
			CEntity* e_enemy = (CEntity*)h_enemy;
			CBTRangedSushi* ranged_sushi = e_enemy->get<CBTRangedSushi>();
			ranged_sushi->setViewDistance(distance);
		}
		//...
	}
}

void CModuleGameController::setHalfConeEnemyByHandle(float half_cone, CHandle h_enemy, int typeEnemy) {
	if (h_enemy.isValid()) {
		if (TYPE_SUSHI == typeEnemy) {
			CEntity* e_enemy = (CEntity*)h_enemy;
			CBTSushi* sushi = e_enemy->get<CBTSushi>();
			sushi->setHalfCone(half_cone);
		}
		else if (TYPE_RANGED_SUSHI == typeEnemy) {
			CEntity* e_enemy = (CEntity*)h_enemy;
			CBTRangedSushi* ranged_sushi = e_enemy->get<CBTRangedSushi>();
			ranged_sushi->setHalfCone(half_cone);
		}
		
	}
}

void CModuleGameController::setPauseEnemyByName(std::string enemy, bool active) {
	
	CEntity* e_enemy = getEntityByName(enemy);
	if (e_enemy != nullptr) {
		TMsgBTPaused msg;
		msg.isPaused = active;
		e_enemy->sendMsg(msg);
	}
}

void CModuleGameController::setPauseEnemyByHandle(CHandle h_enemy, bool active) {

	CEntity* e_enemy = h_enemy;
	if (e_enemy != nullptr) {
		TMsgBTPaused msg;
		msg.isPaused = active;
		e_enemy->sendMsg(msg);
	}
}




void CModuleGameController::setTransformObject(std::string name,VEC3 pos,float yaw, float pith, float roll) {
	if (name != "") {
		CHandle handle = getEntityByName(name);
		CEntity* entity = handle;
		TCompTransform* transform = entity->get<TCompTransform>();
		transform->setPosition(pos);
		transform->setAngles(deg2rad(yaw), deg2rad(pith), deg2rad(roll));

		QUAT quat = transform->getRotation();
		//VEC3_TO_PXVEC3()
		TCompCollider* collider = entity->get<TCompCollider>();
		//collider->setGlobalPose(QUAT_TO_PXQUAT(quat););
		collider->setGlobalPose(pos,quat);

		//transform->setScale(scale);
	} 
}


void CModuleGameController::inCinematic(bool active) {
	CHandle e_player = getEntityByName("Player");
	CEntity* entity = e_player;
	TCompSCartController* scartController = entity->get<TCompSCartController>();
	bool isScart = scartController->isEnabled;
	TMsgOnCinematic msgOnCinematic;
	msgOnCinematic.cinematic = active;
	msgOnCinematic.isscart = isScart;
	e_player.sendMsg(msgOnCinematic);
}

void CModuleGameController::inCinematicSpecial(bool active, int type) {
	CHandle e_player = getEntityByName("Player");
	CEntity* entity = e_player;
	TCompSCartController* scartController = entity->get<TCompSCartController>();
	bool isScart = scartController->isEnabled;
	TMsgOnCinematicSpecial msgOnCinematicSpec;
	msgOnCinematicSpec.cinematic = active;
	msgOnCinematicSpec.isscart = isScart;
	msgOnCinematicSpec.type = type;
	e_player.sendMsg(msgOnCinematicSpec);
}



void CModuleGameController::inCinematicGolem(std::string name, bool active) {
	CHandle e_golem = getEntityByName(name);
	CHandle e_player = getEntityByName("Player");
	CEntity* entity = e_player;
	TCompSCartController* scartController = entity->get<TCompSCartController>();
	
	bool isScart = scartController->isEnabled;
	TMsgOnCinematic msgOnCinematic;
	msgOnCinematic.cinematic = active;
	msgOnCinematic.isscart = isScart;
	if (e_golem.isValid()) {
		e_golem.sendMsg(msgOnCinematic);
	}
}

void CModuleGameController::changeGameState(std::string name) {
	CEngine::get().getModules().changeToGamestate(name);
}


void CModuleGameController::setLifeEnemy(CHandle h,int typeEnemy,float life) {
	CEntity* e_enemy = (CEntity*)h;

	if (typeEnemy == TYPE_CUPCAKE) {
		CBTCupcake* cupcake = e_enemy->get<CBTCupcake>();
		cupcake->setLife(life);
	}
	if (typeEnemy == TYPE_SUSHI) {
		CBTSushi* sushi_n = e_enemy->get<CBTSushi>();
		//sushi_n->setLifesetLife no esta el set en la clase
	}
	if (typeEnemy == TYPE_RANGED_SUSHI) {
		CBTRangedSushi* sushi_r = e_enemy->get<CBTRangedSushi>();
		//sushi_n->setLifesetLife: no esta el set en la clase
	}

}

void CModuleGameController::changeShadowsEnabledJoint(bool value) {
	VHandles v_lightJoints = CTagsManager::get().getAllEntitiesByTag(getID("jointsLight"));
	for (const auto& entity : v_lightJoints) {
		CEntity* e_entity = (CEntity*)entity;
		if (e_entity != nullptr) {
			TCompLightDir* lightsDirComp = e_entity->get<TCompLightDir>();
 			lightsDirComp->setShadowEnabled(value);
		}
	}
	
}

void CModuleGameController::changeLightsIntensityJoint(float value) {
	VHandles v_lightJoints = CTagsManager::get().getAllEntitiesByTag(getID("jointsLight"));
	for (const auto& entity : v_lightJoints) {
		CEntity* e_entity = (CEntity*)entity;
		if (e_entity != nullptr) {
			TCompLightDir* lightsDirComp = e_entity->get<TCompLightDir>();
			lightsDirComp->setIntensity(value);
		}
	}

}



void CModuleGameController::loadScene(const std::string name) {
	SceneManager.getSceneManager()->loadScene(name);
}


void CModuleGameController::loadProducts(std::string zona) {
  TFileContext fc(zona);
  TEntityParseContext ctx;
  CEngine::get().getGPUCulling().parseProducts(zona, ctx);

  EnginePhysics.gScene->forceDynamicTreeRebuild(true, true);
}

CHandle CModuleGameController::entityByName(std::string name) {
	CHandle h = getEntityByName(name);
	return h;
}

void CModuleGameController::pausedPlayer(bool value) {
	CEntity* e_player = getPlayerHandle();
	TCompCharacterController* characterController = e_player->get<TCompCharacterController>();
	characterController->_pausedAI = value;
}


template <typename T>
void CModuleGameController::broadcastMessage(T message) {
	//EngineEntities.
}

//function casts

/*
TCompCurveController* TCurveController(CHandle h)
{
	TCompCurveController* t = h;
	return t;
}*/

CEntity* toEntity(CHandle h)
{
	CEntity* e = h;
	return e;
}
TCompSkelLookAt* toCompSkelLookAt(CHandle h)
{
	TCompSkelLookAt* t = h;
	return t;
}

TCompEnemiesInTube* toCompEnemiesInTube(CHandle h) {
	TCompEnemiesInTube* it = h;
	return it;
	
}

TCompMorphAnimation* toCompMorphAnimation(CHandle h) {
	TCompMorphAnimation* m = h;
	return m;
}

TCompName* toCompName(CHandle h) {
	TCompName* n = h;
	return n;
}

TCompTransform* toCompTransform(CHandle h) {
	TCompTransform* t = h;
	return t;
}

TCompCamera* toCompCamera(CHandle h) {
	TCompCamera* c = h;
	return c;
}

CBTGolem* toCBTGolem(CHandle h) {
	CBTGolem* g = h;
	return g;
}

/*TCompEnemySpawnerSpecialTrap* toCompEnemySpawnerSpecialTrap(CHandle h) {
	TCompEnemySpawnerSpecialTrap* t = h;
	return t;
}*/

TCompSelfDestroy* toCompSelfDestroy(CHandle h) {
	TCompSelfDestroy* s = h;
	return s;
}

CBTCupcake* toCBTCupcake(CHandle h) {
	CBTCupcake* c = h;
	return c;
}

TCompEnemySpawner* toCompEnemySpawner(CHandle h) {
	TCompEnemySpawner* c = h;
	return c;
}

TCompLightPoint* toCompLightPoint(CHandle h) {
	TCompLightPoint* l = h;
	return l;
}

TCompFlickering* toCompFlickering(CHandle h) {
	TCompFlickering* f = h;
	return f;
}

TCompLightDir* toCompLightDir(CHandle h) {
	TCompLightDir* l = h;
	return l;
}


TCompCharacterController* toCompCharacterController_(CHandle h) {
	TCompCharacterController* c = h;
	return c;
}


TCompBalance* toCompBalance(CHandle h) {
	TCompBalance* b = h;
	return b;
}


void CModuleGameController::updateAmbientLight(float amount) {
  CEngine::get().getRender().setNewAmbient(amount);
}

//Soundtrack Functions
void CModuleGameController::startSoundtrack(int track_id = 0) {
    EngineAudio.soundtrack = EngineAudio.playEvent("event:/Music/Soundtrack");
    EngineAudio.soundtrack.setParameter("soundtrack_id", track_id);
}
void CModuleGameController::pauseSoundtrack(){
    EngineAudio.soundtrack.setPaused(true);
}
void CModuleGameController::resumeSoundtrack() {
    EngineAudio.soundtrack.setPaused(false);
}
void CModuleGameController::updateSoundtrackID(int new_track_id = 0) {
    EngineAudio.soundtrack.setParameter("soundtrack_id", new_track_id);
}
void CModuleGameController::setSoundtrackVolume(float volume) {
    if(!EngineAudio.announcement.isPlaying()){
        EngineAudio.soundtrack.setVolume(volume);
    }
}
float CModuleGameController::getSoundtrackVolume() {
    return EngineAudio.soundtrack.getVolume();
}
void CModuleGameController::playAnnouncement(std::string announcement = "") {
    assert(announcement != "");
    setSoundtrackVolume(0.1f);
    EngineAudio.announcement = EngineAudio.playEvent(announcement);
    float audioLength = EngineAudio.announcement.getLength() / 1000.f;
    Scripting.execActionDelayed("setSoundtrackVolume(1.0)", audioLength + 1.f);
}
void CModuleGameController::startAudioPlayer(std::string entity = "") {
    assert(entity != "");
    CEntity* holder = entityByName(entity);
    if (holder) {
        TCompAudioPlayer* player = holder->get< TCompAudioPlayer>();
        if (player)
            player->play();
    }
}
//End Soundtrack Functions

void CModuleGameController::pauseGame() {
    EngineAudio.soundtrack.setPaused(true);
    EngineAudio.secondarySoundtrack.setPaused(false);
    EngineAudio.announcement.setPaused(true);
}

void CModuleGameController::resumeGame() {
    EngineAudio.soundtrack.setPaused(false);
    EngineAudio.secondarySoundtrack.setPaused(true);
    EngineAudio.announcement.setPaused(false);
}

void CModuleGameController::cheatPosition() {
	CEntity* entity = getPlayerHandle();
	TCompTransform* c_trans = entity->get<TCompTransform>();
	TCompCollider* comp_collider = entity->get<TCompCollider>();
	if (!comp_collider || !comp_collider->controller)
		return;

	VEC3 posArea = positionAreas[positionCheat];
	physx::PxExtendedVec3 pxPosArea = VEC3_TO_PXEXVEC3(posArea);
	comp_collider->controller->setPosition(pxPosArea);
	//dbg("POSITION:%i\n", positionCheat);
	if (positionCheat >= positionAreas.size() - 1) {
		positionCheat = 0;
	}
	positionCheat++;
}



void CModuleGameController::deactivateWidget(std::string name) {
	CEngine::get().getUI().deactivateWidgetClass(name);
}

void CModuleGameController::activateWidget(std::string name) {
	CEngine::get().getUI().activateWidgetClass(name);
}


void CModuleGameController::childAppears(std::string name, bool getFromChildren, bool alfaPos, float valueIni, float valueFin) {;
CEngine::get().getUI().activateWidgetClass(name)->childAppears(getFromChildren, alfaPos, valueIni, valueFin);
}


void CModuleGameController::stopWidgetEffect(const std::string& nameWidgetStrMap, const std::string& nameEffect) {
	CEngine::get().getUI().stopWidgetEffect(nameWidgetStrMap, nameEffect);
}

void CModuleGameController::changeSpeedWidgetEffect(const std::string& nameWidgetStrMap, const std::string& nameEffect,float x, float y) {
	CEngine::get().getUI().changeSpeedWidgetEffect(nameWidgetStrMap, nameEffect,  x,  y);
}


void CModuleGameController::stopWidgetEffectSpecial() {
	UI::CModuleUI& ui = Engine.getUI();
	UI::CWidget* widgetPadre = ui.getWidget("INTRO_SCREEN");
	UI::CWidget* widgetHijo = widgetPadre->getChildren(0)->getChildren(0);
	UI::CEffect* effect = widgetHijo->getEffect("effectAnimateComic");
	effect->stopUiFx();
}

void CModuleGameController::changeSpeedWidgetEffectSpecial(float x, float y) {
	UI::CModuleUI& ui = Engine.getUI();
	UI::CWidget* widgetPadre = ui.getWidget("INTRO_SCREEN");
	UI::CWidget* widgetHijo = widgetPadre->getChildren(0)->getChildren(0);
	UI::CEffect* effect = widgetHijo->getEffect("effectAnimateComic");
	effect->changeSpeedUV(x, y);
}

void CModuleGameController::changeDurationWidgetEffectSpecial(float duration) {
	UI::CModuleUI& ui = Engine.getUI();
	UI::CWidget* widgetPadre = ui.getWidget("INTRO_SCREEN");
	UI::CWidget* widgetHijo = widgetPadre->getChildren(0)->getChildren(0);
	UI::CEffect* effect = widgetHijo->getEffect("effectScaleComic");
	effect->changeDuration(duration);
}


void CModuleGameController::resurrectionInGameOver() {
	CEntity* e_player = getEntityByName("Player");
	if (!e_player) {
		return;
	}
	TCompCharacterController* c_controller = e_player->get<TCompCharacterController>();
	if ((c_controller->life <= 0)) {

		//quitar puntero de raton
		TMsgGamePause msg;
		msg.isPause = false;
		msg.playerDead = false;
		CEntity* cam_player = getEntityByName("PlayerCamera");
		if (cam_player != nullptr) {
			cam_player->sendMsg(msg);
		}
		//Scripting.execActionDelayed("loadCheckpoint()", 2.0);
		//Scripting.execActionDelayed("changeGameState(\"gs_gameplay\")", 2.0);
		GameController.loadCheckpoint();
		CEngine::get().getModules().changeToGamestate("gs_gameplay");
		
		c_controller->ChangeState("GROUNDED");
	}
	
}

void CModuleGameController::exitGame() {
	auto& app = CApplication::get();
	DestroyWindow(app.getHandle());
}


void CModuleGameController::setBloomInCam(bool value) {
	CEntity* cam_player = getEntityByName("MainCamera");
	TCompRenderBloom* renderBloom = cam_player->get<TCompRenderBloom>();
	renderBloom->enabled = value;
}



CCamera* CModuleGameController::getCameraFromHandle(CHandle hCamera)
{
	if (hCamera.isValid())
	{
		CEntity* e = hCamera;
		TCompCamera* cCamera = e->get<TCompCamera>();
		if (cCamera)
		{
			return cCamera;
		}
	}
	return nullptr;
}






