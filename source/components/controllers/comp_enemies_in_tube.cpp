#include "mcv_platform.h"
#include "comp_enemies_in_tube.h"
#include "components/controllers/character/comp_character_controller.h"
#include "modules/game/module_game_controller.h"
#include "engine.h"
#include "components/ai/bt/bt_cupcake.h"
#include "components/objects/comp_rotator.h"

DECL_OBJ_MANAGER("enemies_in_tube", TCompEnemiesInTube);

void TCompEnemiesInTube::debugInMenu() {

}


void TCompEnemiesInTube::registerMsgs() {
  DECL_MSG(TCompEnemiesInTube, TMSgWallDestroyed, onFinishEnemiesInTube);
}

void TCompEnemiesInTube::onFinishEnemiesInTube(const TMSgWallDestroyed& msg) {
  if (msg.isDetroyed) {
    activateTrap = false;
    CHandle(this).getOwner().destroy();
    CHandle(this).destroy();
  }
}


void TCompEnemiesInTube::load(const json& j, TEntityParseContext& ctx) {
  enemiesPosition.push_back(VEC3(18.214, 9.143, -7.335));
  enemiesPosition.push_back(VEC3(27.714, 9.143, -7.335));
  enemiesPosition.push_back(VEC3(27.714, 9.143, 5.790));
  enemiesPosition.push_back(VEC3(18.064, 9.143, 5.890));
}


void TCompEnemiesInTube::update(float dt) {

  if (activateTrap) {
    if (birthTime > 0.0f) {
      birthTime -= dt;
      if (birthTime < 4.5f) {
        CEntity* entity_emis1 = (CEntity*)h1;
        if (entity_emis1 != nullptr) {
          TCompBuffers* c_buff1 = entity_emis1->get<TCompBuffers>();
          if (c_buff1) {
            auto buf1 = c_buff1->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data1 = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf1);
            data1->emitter_num_particles_per_spawn = 25;
            data1->updateGPU();

          }
        }
        CEntity* entity_emis2 = (CEntity*)h2;
        if (entity_emis2 != nullptr) {
          TCompBuffers* c_buff2 = entity_emis2->get<TCompBuffers>();
          if (c_buff2) {
            auto buf2 = c_buff2->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data2 = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf2);
            data2->emitter_num_particles_per_spawn = 25;
            data2->updateGPU();
          }
        }
        CEntity* entity_emis3 = (CEntity*)h3;
        if (entity_emis3 != nullptr) {
          TCompBuffers* c_buff3 = entity_emis3->get<TCompBuffers>();
          if (c_buff3) {
            auto buf3 = c_buff3->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data3 = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf3);
            data3->emitter_num_particles_per_spawn = 25;
            data3->updateGPU();
          }
        }
        CEntity* entity_emis4 = (CEntity*)h4;
        if (entity_emis4 != nullptr) {
          TCompBuffers* c_buff4 = entity_emis4->get<TCompBuffers>();
          if (c_buff4) {
            auto buf4 = c_buff4->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data4 = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf4);
            data4->emitter_num_particles_per_spawn = 25;
            data4->updateGPU();
          }
        }
      }
    }
    else {

      if (positionBirth == 0) {
        if (!activateTrap1) {
          h1 = GameController.spawnPrefab("data/particles/smoke_tube1.json", VEC3(), QUAT(0, 0, 0, 1), 1);
          activateTrap1 = true;
		  CEntity* palaBatidora1 = GameController.entityByName("palaBatidora1");
		  TCompRotator* rot1 = palaBatidora1->get<TCompRotator>();
		  CEntity* palaBatidora2 = GameController.entityByName("palaBatidora2");
		  TCompRotator* rot2 = palaBatidora2->get<TCompRotator>();
		  rot1->setActivateRotator(true);
		  rot2->setActivateRotator(true);
        }
        else {
          CEntity* entity_emis = (CEntity*)h1;
          TCompBuffers* c_buff = entity_emis->get<TCompBuffers>();
          if (c_buff) {
            auto buf = c_buff->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
            data->emitter_num_particles_per_spawn = 25;
            data->updateGPU();
          }
        }
      }
      if (positionBirth == 1) {
        if (!activateTrap2) {
          h2 = GameController.spawnPrefab("data/particles/smoke_tube3.json", VEC3(), QUAT(0, 0, 0, 1), 1);
          activateTrap2 = true;
		  CEntity* palaBatidora3 = GameController.entityByName("palaBatidora3");
		  TCompRotator* rot3 = palaBatidora3->get<TCompRotator>();
		  CEntity* palaBatidora4 = GameController.entityByName("palaBatidora4");
		  TCompRotator* rot4 = palaBatidora4->get<TCompRotator>();
		  rot3->setActivateRotator(true);
		  rot4->setActivateRotator(true);
        }
        else {
          CEntity* entity_emis = (CEntity*)h2;
          TCompBuffers* c_buff = entity_emis->get<TCompBuffers>();
          if (c_buff) {
            auto buf = c_buff->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
            data->emitter_num_particles_per_spawn = 25;
            data->updateGPU();
          }
        }
      }
      if (positionBirth == 2) {
        if (!activateTrap3) {
          h3 = GameController.spawnPrefab("data/particles/smoke_tube2.json", VEC3(), QUAT(0, 0, 0, 1), 1);
          activateTrap3 = true;
		  CEntity* palaBatidora5 = GameController.entityByName("palaBatidara5");
		  TCompRotator* rot5 = palaBatidora5->get<TCompRotator>();
		  CEntity* palaBatidora6 = GameController.entityByName("palaBatidora6");
		  TCompRotator* rot6 = palaBatidora6->get<TCompRotator>();
		  rot5->setActivateRotator(true);
		  rot6->setActivateRotator(true);
        }
        else {
          CEntity* entity_emis = (CEntity*)h3;
          TCompBuffers* c_buff = entity_emis->get<TCompBuffers>();
          if (c_buff) {
            auto buf = c_buff->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
            data->emitter_num_particles_per_spawn = 25;
            data->updateGPU();
          }
        }
      }
      if (positionBirth == 3) {
        if (!activateTrap4) {
          h4 = GameController.spawnPrefab("data/particles/smoke_tube4.json", VEC3(), QUAT(0, 0, 0, 1), 1);
          activateTrap4 = true;
		  CEntity* palaBatidora8 = GameController.entityByName("palaBatidora8");
		  TCompRotator* rot8 = palaBatidora8->get<TCompRotator>();
		  CEntity* palaBatidora7 = GameController.entityByName("palaBatidora7");
		  TCompRotator* rot7 = palaBatidora7->get<TCompRotator>();
		  rot7->setActivateRotator(true);
		  rot8->setActivateRotator(true);
        }
        else {
          CEntity* entity_emis = (CEntity*)h4;
          TCompBuffers* c_buff = entity_emis->get<TCompBuffers>();
          if (c_buff) {
            auto buf = c_buff->getCteByName("TCtesParticles");
            CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
            data->emitter_num_particles_per_spawn = 25;
            data->updateGPU();
          }
        }
      }
      CHandle handle = GameController.spawnPrefab("data/prefabs/enemies/bt_cupcake.json", enemiesPosition[positionBirth], QUAT(0, 0, 0, 1), 1);

      if (!firstTime) {
        /*GameController.spawnPrefab("data/particles/smoke_tube1.json", VEC3(), QUAT(0, 0, 0, 1), 1);
        GameController.spawnPrefab("data/particles/smoke_tube2.json", VEC3(), QUAT(0, 0, 0, 1), 1);
        GameController.spawnPrefab("data/particles/smoke_tube3.json", VEC3(), QUAT(0, 0, 0, 1), 1);
        GameController.spawnPrefab("data/particles/smoke_tube4.json", VEC3(), QUAT(0, 0, 0, 1), 1);*/

		  GameController.startAudioPlayer("1");//batidora 1
		  GameController.startAudioPlayer("2");//batidora 2
		  GameController.startAudioPlayer("3");//batidora 3
		  GameController.startAudioPlayer("4");// batidora 4
		  GameController.startAudioPlayer("005");//batidora 1
		  GameController.startAudioPlayer("006");//batidora 2
		  GameController.startAudioPlayer("007");//batidora 3
		  GameController.startAudioPlayer("008");// batidora 4
		  GameController.startAudioPlayer("009");//batidora 1
		  GameController.startAudioPlayer("010");//batidora 2
		  GameController.startAudioPlayer("011");//batidora 3
		  GameController.startAudioPlayer("012");// batidora 4
		  GameController.startAudioPlayer("013");//batidora 1
		  GameController.startAudioPlayer("014");//batidora 2
		  GameController.startAudioPlayer("015");//batidora 3
		  GameController.startAudioPlayer("016");// batidora 4

		  

        firstTime = true;
      }

      CEntity* entity = (CEntity*)handle;
      CBTCupcake* cupcake = entity->get<CBTCupcake>();
      cupcake->setHalfCone(360);
      cupcake->setLengthCone(300);
      positionBirth++;
      if (positionBirth >= enemiesPosition.size()) {
        positionBirth = 0;
      }
      birthTime = birthTimeReset;
    }
  }

}


void TCompEnemiesInTube::setActivateTrap(bool value) {
	activateTrap = value;
}



//CODIGO ANTIGUO

/*
#include "mcv_platform.h"
#include "comp_enemies_in_tube.h"
#include "components/controllers/character/comp_character_controller.h"
#include "modules/game/module_game_controller.h"
#include "engine.h"
#include "components/ai/bt/bt_cupcake.h"
DECL_OBJ_MANAGER("enemies_in_tube", TCompEnemiesInTube);

void TCompEnemiesInTube::debugInMenu() {

}


void TCompEnemiesInTube::registerMsgs() {
  DECL_MSG(TCompEnemiesInTube, TMSgWallDestroyed, onFinishEnemiesInTube);
}

void TCompEnemiesInTube::onFinishEnemiesInTube(const TMSgWallDestroyed & msg) {
  if (msg.isDetroyed) {
    activateTrap = false;
    CHandle(this).getOwner().destroy();
    CHandle(this).destroy();
  }
}


void TCompEnemiesInTube::load(const json& j, TEntityParseContext& ctx){
  enemiesPosition.push_back(VEC3(18.214,9.143,-7.335));
  enemiesPosition.push_back(VEC3(27.714, 9.143, -7.335));
  enemiesPosition.push_back(VEC3(27.714, 9.143, 5.790));
  enemiesPosition.push_back(VEC3(18.064, 9.143, 5.890));



}


void TCompEnemiesInTube::update(float dt) {
  if(activateTrap){
    if (birthTime > 0) {
      birthTime -= dt;
    }
    else {
      CHandle handle = GameController.spawnPrefab("data/prefabs/enemies/bt_cupcake.json", enemiesPosition[positionBirth], QUAT(0, 0, 0, 1), 1);
      if(!firstTime){
        GameController.spawnPrefab("data/particles/smoke_tube1.json", VEC3(), QUAT(0, 0, 0, 1), 1);
        GameController.spawnPrefab("data/particles/smoke_tube2.json", VEC3(), QUAT(0, 0, 0, 1), 1);
        GameController.spawnPrefab("data/particles/smoke_tube3.json", VEC3(), QUAT(0, 0, 0, 1), 1);
        GameController.spawnPrefab("data/particles/smoke_tube4.json", VEC3(), QUAT(0, 0, 0, 1), 1);
        firstTime = true;
      }

      CEntity* entity = (CEntity *)handle;
      CBTCupcake* cupcake = entity->get<CBTCupcake>();
      cupcake->setHalfCone(360);
      cupcake->setLengthCone(300);
      positionBirth++;
      if (positionBirth >= enemiesPosition.size() ) {
        positionBirth = 0;
      }
      birthTime = birthTimeReset;
    }
  }
}

*/