#include "mcv_platform.h"
#include "engine.h"
#include "components/common/comp_transform.h"
#include "components/objects/comp_termoestato.h"


DECL_OBJ_MANAGER("comp_termoestato", TCompTermoestato);

void TCompTermoestato::debugInMenu() {



}

void TCompTermoestato::load(const json& j, TEntityParseContext& ctx) {
	
}

void TCompTermoestato::registerMsgs() {
	DECL_MSG(TCompTermoestato, TMsgGravity, onBattery);
}

void TCompTermoestato::update(float delta) {
	if (!_isEnabled) {
		TCompCollider* c_collider = get<TCompCollider>();
		physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);
		if (c_collider != nullptr) {
			
			#ifndef NDEBUG
				return;
			#endif

			/*--OK--*/
			rigid_dynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
			rigid_dynamic->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true); //en release OK
			rigid_dynamic->addForce(physx::PxVec3(0,0,0), physx::PxForceMode::eFORCE); // en release OK
			/*-----*/
		}
	}
	_isEnabled = true;
}




void TCompTermoestato::onBattery(const TMsgGravity & msg) {
	
    EngineAudio.playEvent("event:/Character/Powers/Battery/Glitch");
	//llamar a funcion de fragment
	int n = GameController.getNumThermoStatesActives();
	if(n == 1) {
		GameController.activateSecondEvent();
	}
	else{
		GameController.activateGameplayFragment("ambush");
	}
   //Animate or start particle system, do something
    /*
	if (!working) { //cambiar esto a u posible nuevo componenete termometro
		
		CHandle h_player = GameController.getPlayerHandle();
		CEntity* e_player = (CEntity *)h_player;
		TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
		c_bb->incrementTermostateBroken();
		

	}
	*/
	//CHandle(this).getOwner().destroy();
	//CHandle(this).destroy();

	TCompCollider* c_collider = get<TCompCollider>();
	physx::PxRigidDynamic* rigid_dynamic = static_cast<physx::PxRigidDynamic*>(c_collider->actor);
	#ifndef NDEBUG
		rigid_dynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
		CHandle(this).destroy();
		return;	
	#endif
	
	/*  OK  */
	rigid_dynamic->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);//en release OK
	rigid_dynamic->addForce(physx::PxVec3(0, 9.81, 0), physx::PxForceMode::eFORCE);//en release OK
	CHandle(this).destroy();
	/* -------    */
}
	

