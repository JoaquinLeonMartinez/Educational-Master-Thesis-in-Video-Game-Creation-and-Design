#include "mcv_platform.h"
#include "engine.h"
#include "module_scripting.h"
#include "input/devices/device_mouse.h"
#include "components/common/comp_tags.h"
#include "components/controllers/character/comp_character_controller.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "windows/app.h"
#include "components/objects/checkpoints/checkpoint.h"
#include "components/common/comp_tags.h"
#include "input/module_input.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_transform.h"
#include "skeleton/comp_skel_lookat.h"
#include <experimental/filesystem>
#include "SLB/include/SLB/SLB.hpp"
#include "components/ai/bt/bt_golem.h"

SLB::Manager* m = new SLB::Manager;
SLB::Script* s = new SLB::Script(m);


bool CModuleScripting::start() {
	
	doBingings();
	
	try {
		loadScriptsInFolder("data/scripts"); 

	}
	catch (exception e) {
    fatal("error starting lua\n");
	}

    return true;
}

/* Load all scripts.lua in given path and its subfolders */
void CModuleScripting::loadScriptsInFolder(const char * path){
	try {
		if (std::experimental::filesystem::exists(path) && std::experimental::filesystem::is_directory(path)) {

			std::experimental::filesystem::recursive_directory_iterator iter(path);
			std::experimental::filesystem::recursive_directory_iterator end;

			while (iter != end) {
				std::string fileName = iter->path().string();
				if (iter->path().extension().string() == ".lua" &&
					!std::experimental::filesystem::is_directory(iter->path())) {
					dbg("File : %s loaded\n", fileName.c_str());
					s->doFile(fileName);
				}
				std::error_code ec;
				iter.increment(ec);
				if (ec) {
					fatal("Error while accessing %s: %s\n", iter->path().string().c_str(), ec.message().c_str());
				}
			}
		}
	}
	catch (std::system_error & e) {
		fatal("Exception %s while loading scripts\n", e.what());
	}
}

void CModuleScripting::update(float delta) {
	//delayed actions
	for (int i = delayedActions.size() - 1; i >= 0; i--) {
		delayedActions[i].time -= delta;
		if (delayedActions[i].time <= 0) {
			std::string aux_call = delayedActions[i].action;
			delayedActions.erase(delayedActions.begin() + i);
			execAction(aux_call);
		}
	}

}

void CModuleScripting::stop()
{

}

void CModuleScripting::doBingings() {

	BindGameController();
	BindCharacterController();
	BindGeometry();
	BindGlobalFunctions();
	BindSkeleton();
	BindHandle();
	BindTransform();
	BindCamera();
	BindConverters();
	BindEnemiesInTube();
	BindName();
	BindGolem();
	BindEnemySpawner();
	BindPointLights();
	BindFlickering();
	BindCharacterController();
	BindBalanceo();
	BindDirectionalLights();
}



void CModuleScripting::BindGameController() {
	SLB::Class<CModuleGameController>("GameController", m)
		.comment("This is our wrapper of the Game Controller")
		.set("healPlayer", &CModuleGameController::healPlayer)//funciones a las que puedes acceder desde lua
		.set("restoreMadness", &CModuleGameController::restoreMadness)
		.set("spawnPrefab", &CModuleGameController::spawnPrefab)
		.set("spawnPrefaByPoint", &CModuleGameController::spawnPrefabByPoint)
		.set("setDamage", &CModuleGameController::setDamage)
		.set("setGodMode", &CModuleGameController::setGodMode)
		.set("stopEnemies", &CModuleGameController::stopBehaviorTrees)
		.set("resumeEnemies", &CModuleGameController::resumeBehaviorTrees)
		.set("activateGameplayFragment", &CModuleGameController::activateGameplayFragment)
		.set("deactivateGameplayFragment", &CModuleGameController::deactivateGameplayFragment)
		.set("wakeUpSushi", &CModuleGameController::wakeUpSushi)
		.set("deleteElement", &CModuleGameController::deleteElement)
		.set("activateSpawner", &CModuleGameController::activateSpawner)
		.set("updateEnemyCurveByName", &CModuleGameController::updateEnemyCurveByName)
		.set("updateEnemyCurveByHandle", &CModuleGameController::updateEnemyCurveByHandle)
		.set("updateCupcakeCurveByHandle", &CModuleGameController::updateCupcakeCurveByHandle)
		.set("setPauseEnemyByName", &CModuleGameController::setPauseEnemyByName)
		.set("destroyWallByName", &CModuleGameController::destroyWallByName)
		.set("wakeUpGolem", &CModuleGameController::wakeUpGolem)
		.set("sleepGolem", &CModuleGameController::sleepGolem)
		.set("setPauseEnemyByHandle", &CModuleGameController::setPauseEnemyByHandle)
		.set("blendingCamera", &CModuleGameController::blendingCamera)
		.set("lockCamera3Person", &CModuleGameController::lockCamera3Person)
		.set("destroyCHandleByName", &CModuleGameController::destroyCHandleByName)
		.set("activatePlatformByName", &CModuleGameController::activatePlatformByName)
		.set("setTransformObject", &CModuleGameController::setTransformObject)
		.set("resetCamera", &CModuleGameController::resetCamera)
		.set("inCinematic", &CModuleGameController::inCinematic)
		.set("inCinematicGolem", &CModuleGameController::inCinematicGolem)
		.set("getPlayerHandle", &CModuleGameController::getPlayerHandle)
    .set("loadScene",&CModuleGameController::loadScene)
    .set("updatePlatformCurveByName", &CModuleGameController::updatePlatformCurveByName)
		.set("playAnimationMorph", &CModuleGameController::playAnimationMorph)
        .set("stopAnimationMorph", &CModuleGameController::stopAnimationMorph)
        .set("startSoundtrack", &CModuleGameController::startSoundtrack)
        .set("pauseSoundtrack", &CModuleGameController::pauseSoundtrack)
        .set("resumeSoundtrack", &CModuleGameController::resumeSoundtrack)
        .set("updateSoundtrackID", &CModuleGameController::updateSoundtrackID)
        .set("setSoundtrackVolume", &CModuleGameController::setSoundtrackVolume)
        .set("playAnnouncement", &CModuleGameController::playAnnouncement)
        .set("startAudioPlayer", &CModuleGameController::startAudioPlayer)
    .set("setAmbient", &CModuleGameController::updateAmbientLight)
		.set("entityByName", &CModuleGameController::entityByName)
		.set("dbgInLua", &CModuleGameController::dbgInLua)
		.set("setHeightEnemyByHandle", &CModuleGameController::setHeightEnemyByHandle)
		.set("saveCheckpoint", &CModuleGameController::saveCheckpoint)
		.set("setViewDistanceEnemyByHandle",&CModuleGameController::setViewDistanceEnemyByHandle)
		.set("getCameraFromHandle", &CModuleGameController::getCameraFromHandle)
		.set("inCinematicSpecial", &CModuleGameController::inCinematicSpecial)
		.set("blendPlayerCamera", &CModuleGameController::blendPlayerCamera)
		.set("setHalfConeEnemyByHandle", &CModuleGameController::setHalfConeEnemyByHandle)
		.set("deleteByTag", &CModuleGameController::deleteByTag)
		.set("deleteCupcake", &CModuleGameController::deleteCupcake)
		.set("deleteSushi", &CModuleGameController::deleteSushi)
		.set("deleteGolem", &CModuleGameController::deleteGolem)
		.set("setLifeEnemy", &CModuleGameController::setLifeEnemy)
		.set("setLifeEnemiesByTag", &CModuleGameController::setLifeEnemiesByTag)
		.set("changeGameState", &CModuleGameController::changeGameState)
		.set("deactivateWidget", &CModuleGameController::deactivateWidget)
		.set("activateWidget", &CModuleGameController::activateWidget)
		.set("childAppears", &CModuleGameController::childAppears)
		.set("loadCheckpoint", &CModuleGameController::loadCheckpoint)
		.set("changeShadowsEnabledJoint", &CModuleGameController::changeShadowsEnabledJoint)
		.set("changeLightsIntensityJoint", &CModuleGameController::changeLightsIntensityJoint)
		.set("changeLightsIntensityJoint", &CModuleGameController::changeLightsIntensityJoint)
		.set("resurrectionInGameOver", &CModuleGameController::resurrectionInGameOver)
		.set("stopWidgetEffect", &CModuleGameController::stopWidgetEffect)
		.set("stopWidgetEffectSpecial", &CModuleGameController::stopWidgetEffectSpecial)
		.set("changeSpeedWidgetEffectSpecial", &CModuleGameController::changeSpeedWidgetEffectSpecial)
		.set("changeDurationWidgetEffectSpecial", &CModuleGameController::changeDurationWidgetEffectSpecial)
		.set("changeSpeedWidgetEffect", &CModuleGameController::changeSpeedWidgetEffect)
		.set("exitGame", &CModuleGameController::exitGame)
		.set("setResurrect", &CModuleGameController::setResurrect)
		.set("setBloomInCam", &CModuleGameController::setBloomInCam)
		.set("pausedPlayer", &CModuleGameController::pausedPlayer)
		;
}

void CModuleScripting::BindConverters() {
	//PUEDE ESTAR MAL
	m->set("toEntity", SLB::FuncCall::create(&toEntity));
	m->set("toCompSkelLookAt", SLB::FuncCall::create(&toCompSkelLookAt));
	m->set("toCompMorphAnimation", SLB::FuncCall::create(&toCompMorphAnimation));//toCompMorphAnimation
	m->set("toCompEnemiesInTube", SLB::FuncCall::create(&toCompEnemiesInTube));
	m->set("toCompName", SLB::FuncCall::create(&toCompName));
	m->set("toCompTransform", SLB::FuncCall::create(&toCompTransform));
	m->set("toCompCamera", SLB::FuncCall::create(&toCompCamera));
	m->set("toCBTGolem", SLB::FuncCall::create(&toCBTGolem));
	//m->set("toCompEnemySpawnerSpecialTrap", SLB::FuncCall::create(&toCompEnemySpawnerSpecialTrap));
	m->set("toCompSelfDestroy", SLB::FuncCall::create(&toCompSelfDestroy));
	m->set("toCBTCupcake", SLB::FuncCall::create(&toCBTCupcake));
	m->set("toCompEnemySpawner", SLB::FuncCall::create(&toCompEnemySpawner));
	m->set("toCompLightPoint", SLB::FuncCall::create(&toCompLightPoint));
	m->set("toCompFlickering", SLB::FuncCall::create(&toCompFlickering));
	m->set("toCompCharacterController_", SLB::FuncCall::create(&toCompCharacterController_));
	m->set("toCompBalance", SLB::FuncCall::create(&toCompBalance));
	m->set("toCompLightDir", SLB::FuncCall::create(&toCompLightDir));
	//toCBTGolem
	//m->set("toCompCharacterController", SLB::FuncCall::create(&toCompCharacterController));
}

void CModuleScripting::BindCharacterController() {
	
	SLB::Class<TCompCharacterController>("CharacterController", m)
		.comment("This is our wrapper of the Player class")
		.set("heal", &TCompCharacterController::heal)
		.property("endgame", &TCompCharacterController::endGame)
	    //.set("changeState",&TCompCharacterController::ChangeState);
		;

	
}
void CModuleScripting::BindSkeleton() {
	//bin componente lookAtSkeleton
	SLB::Class<TCompSkelLookAt>("TCompSkelLookAt", m)
		.comment("This is our wrapper of lookAtSkeleton class")
		.property("h_skeleton", &TCompSkelLookAt::h_skeleton)
		.property("h_target_entity", &TCompSkelLookAt::h_target_entity)
		.property("target_entity_name", &TCompSkelLookAt::target_entity_name)
		.property("amount", &TCompSkelLookAt::amount)
		.property("target_transition_factor", &TCompSkelLookAt::target_transition_factor)
		.property("flagFirst", &TCompSkelLookAt::flagFirst)
		;
}


void CModuleScripting::BindPointLights() {
	SLB::Class<TCompLightPoint>("TCompLightPoint", m)
		.comment("This is our wrapper of point lights class")
		.set("setIntensity", &TCompLightPoint::setIntensity)
		;
}


void CModuleScripting::BindDirectionalLights() {
	SLB::Class<TCompLightDir>("TCompLightDir", m)
		.comment("This is our wrapper of lights class")
		.set("setIntensity", &TCompLightDir::setIntensity)
		.set("setShadowEnabled", &TCompLightDir::setShadowEnabled)
		;
}


void CModuleScripting::BindFlickering() {
	SLB::Class<TCompFlickering>("TCompFlickering", m)
		.comment("This is our wrapper of flickering class")
		.set("setFrequency", &TCompFlickering::setFrequency)
		.set("setBase", &TCompFlickering::setBase)
		.set("setAmplitud", &TCompFlickering::setAmplitud)
		.set("setPhase", &TCompFlickering::setPhase)
		;
}





void CModuleScripting::BindName() {
	SLB::Class<TCompName>("TCompName", m)
		.comment("This is our wrapper of compName class")
		.constructor()
		.set("setName", &TCompName::setName)
		.set("getName", &TCompName::getName)
		;
}


void CModuleScripting::BindGolem() {
	SLB::Class<CBTGolem>("CBTGolem", m)
		.comment("This is our wrapper of compGolem class")
		.constructor()
		.set("setNotThrowCupcake", &CBTGolem::setNotThrowCupcake)
		;
}



/*
void CModuleScripting::BindEnemySpawnerSpecial() {
	SLB::Class<TCompEnemySpawnerSpecialTrap>("TCompEnemySpawnerSpecialTrap", m)
		.comment("This is ouTCompEnemySpawnerSpecialTrapr wrapper of TCompEnemySpawnerSpecialTrap class")
		.constructor()
		.property("working", &TCompEnemySpawnerSpecialTrap::working)
		.set("setSpawnDelay", &TCompEnemySpawnerSpecialTrap::setSpawnDelay)
		;
}*/


void CModuleScripting::BindEnemySpawner() {
	SLB::Class<TCompEnemySpawner>("TCompEnemySpawner", m)
		.comment("This is ouTCompEnemySpawnerSpecialTrapr wrapper of TCompEnemySpawnerSpecialTrap class")
		.constructor()
		.set("setLifeSpawner", &TCompEnemySpawner::setLifeSpawner)
		.set("setScriptTriggerActivate", &TCompEnemySpawner::setScriptTriggerActivate)
		.set("setCurveForSpawner", &TCompEnemySpawner::setCurveForSpawner)
		.set("setComportamentNormal", &TCompEnemySpawner::setComportamentNormal)
		.set("setSpawnDelay", &TCompEnemySpawner::setSpawnDelay)
		.set("setSpawnMaxNumber", &TCompEnemySpawner::setSpawnMaxNumber)
		.property("working", &TCompEnemySpawner::working)
		;
}




void CModuleScripting::BindEnemiesInTube() {
	SLB::Class<TCompEnemiesInTube>("TCompEnemiesInTube", m)
		.comment("This is our wrapper of comp_enemies_in_tube class")
		.set("setActivateTrap", &TCompEnemiesInTube::setActivateTrap)
		.property("activateTrap", &TCompEnemiesInTube::activateTrap)

		/*
		float birthTime = 5.f;
		float birthTimeReset = 5.f;
		float limitTime = 0.f;
		int positionBirth = 0;
		bool activateTrap = false;
		
		*/
		;
}



void CModuleScripting::BindGeometry() {
	//Geometry
	SLB::Class<VEC3>("VEC3", m)
		.comment("This is our wrapper of the Vector3 class")
		.constructor<float, float, float>()
		.property("x", &VEC3::x)
		.property("y", &VEC3::y)
		.property("z", &VEC3::z)
		;


	SLB::Class< QUAT >("QUAT", m)
		.constructor<float, float, float, float>()
		.comment("This is our wrapper of the QUAT class")
		.property("x", &QUAT::x)
		.property("y", &QUAT::y)
		.property("z", &QUAT::z)
		.property("w", &QUAT::w);
}


void CModuleScripting::BindHandle() {
	SLB::Class <CHandle>("CHandle", m)
	.comment("CHandle wrapper")
	.constructor()
	.set("fromUnsigned", &CHandle::fromUnsigned)
	.set("destroy", &CHandle::destroy)
	.set("isValid", &CHandle::isValid);


	SLB::Class <CEntity>("CEntity", m)
		.comment("CEntity wrapper")
		.set("getCompByName", &CEntity::getCompByName);
}

void CModuleScripting::BindTransform() {
	SLB::Class <TCompTransform>("TCompTransform", m)
		.comment("CTransform wrapper")
		.constructor()
		.set("setPosition", &TCompTransform::setPosition)
		.set("getPosition", &TCompTransform::getPosition)
		.set("setRotation", &TCompTransform::setRotation)
		.set("getRotation", &TCompTransform::getRotation)
		.set("lookAt", &TCompTransform::lookAt)
		.set("getDeltaYawToAimTo", &TCompTransform::getDeltaYawToAimTo)
		.set("getScale", &TCompTransform::getScale)
		.set("setScale", &TCompTransform::setScale);
}


void CModuleScripting::BindSelfDestroy() {
	SLB::Class <TCompSelfDestroy>("TCompSelfDestroy", m)
		.comment("TCompSelfDestroy wrapper")
		.constructor()
		.set("setEnabled", &TCompSelfDestroy::setEnabled);
}

void CModuleScripting::BindCamera() {
	SLB::Class <TCompCamera>("TCompCamera", m)
		.comment("TCompCamera wrapper")
		.constructor()
		.set("lookAt", &TCompCamera::lookAt);
}



void CModuleScripting::BindBalanceo() {
	SLB::Class <TCompBalance>("TCompBalance", m)
		.comment("TCompCamera wrapper")
		.constructor()
		.set("balanceo", &TCompBalance::balanceo)
		.set("cambioTexturaJoint", &TCompBalance::cambioTexturaJoint);
}



void CModuleScripting::BindGlobalFunctions() {
	
	//General:
	m->set("salute", SLB::FuncCall::create(&saludar));
	m->set("getGameController", SLB::FuncCall::create(&getGameController));

	//uso de execActionDelayed en scripting
	m->set("execDelayedAction", SLB::FuncCall::create(&execDelayedAction));
}




void CModuleScripting::runScript(std::string scriptType, const std::string& params, float delay) {
	std::string actionToExecute = scriptType;
	
	if (delay == 0.f) {
		execAction(actionToExecute + "(\"" + params + "\")");
	}

	else {
		execActionDelayed(actionToExecute + "(\"" + params + "\")", delay);
	}
	
}


bool CModuleScripting::execEvent(Events event, const std::string & params, float delay) {
	switch (event) {
	case Events::TRIGGER_ENTER:
		if (delay > 0) {
			return execActionDelayed(params, delay);
		}
		else {
			
			return execAction(params);
		}
		break;
	case Events::TRIGGER_EXIT:
		if (delay > 0) {
			return execActionDelayed(params, delay);
		}
		else {
			return execAction(params);
		}
		break;
	}
}




bool CModuleScripting::execAction(const std::string& action) {
	try {
		//dbg("-------->in execAction: %s\n",action.c_str());
		s->doString(action);
		return true;
	}
	catch (std::runtime_error &err) {
		dbg(err.what());
	}

}

bool CModuleScripting::execActionDelayed(const std::string & action, float delay){
	delayedActions.push_back(DelayedAction{ action, delay });
	return true;
}

void immortal(bool active) {
	GameController.setGodMode(active);
}

void saludar() {
	dbg("Saludamos desde LUA amigo");
}

void execDelayedAction(const std::string &action, float delay) {
	//execActionDelayed(action,delay);
	Scripting.execActionDelayed(action, delay);
}

CModuleGameController* getGameController() {
	return GameController.getPointer();	
}

