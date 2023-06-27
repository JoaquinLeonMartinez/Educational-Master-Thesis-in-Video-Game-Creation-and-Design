#include "mcv_platform.h"
#include "module_test_cameras.h"
#include "engine.h"
#include "input/input.h"
#include "modules/module_camera_mixer.h"
#include "geometry/curve.h"
#include "components/common/comp_transform.h"
#include "components/controllers/comp_curve_controller.h"
#include "components/controllers/camera/comp_camera_3rd_person.h"
#include "components/controllers/camera/comp_camera_flyover.h"
#include "components/controllers/character/comp_character_controller.h"

CModuleTestCameras::CModuleTestCameras(const std::string& name)
  : IModule(name)
{}

bool CModuleTestCameras::start()
{
  Engine.getCameraMixer().setDefaultCamera(getEntityByName("PlayerCamera"));
  Engine.getCameraMixer().setOutputCamera(getEntityByName("MainCamera"));

  const float kBlendTime = 1.f;
  static Interpolator::TQuadInOutInterpolator quadInt;

  CHandle hCamera = getEntityByName("PlayerCamera");//PlayerCamera vs DebugCamera
  Engine.getCameraMixer().blendCamera(hCamera, 0.f, &quadInt);

  return true;
}

void CModuleTestCameras::stop()
{
}

void CModuleTestCameras::update(float dt)
{
  if (EngineInput[VK_F1].justPressed()) {
    targetCamera++;
    const float kBlendTime = 1.f;
    static Interpolator::TQuadInOutInterpolator quadInt;
    CEntity* e_camera = getEntityByName("PlayerCamera");
    CEntity* e_camera2 = getEntityByName("DebugCamera");

    CEntity* e_player = getEntityByName("Player");
    TCompCharacterController* c_player = e_player->get<TCompCharacterController>();

    if (targetCamera % 2 == 1) { //PLAYER CAMERA
      Engine.getCameraMixer().blendCamera(e_camera, kBlendTime, &quadInt);
      TCompCamera3rdPerson* c_cam1 = e_camera->get<TCompCamera3rdPerson>();
      TCompCameraFlyover* c_cam2 = e_camera2->get<TCompCameraFlyover>();
      c_cam1->_enabled = true;
      c_cam2->_enabled = false;
      c_player->enabled = true;
    }
    else { //DEBUG CAMERA
      Engine.getCameraMixer().blendCamera(e_camera2, kBlendTime, &quadInt);
      TCompCamera3rdPerson* c_cam1 = e_camera->get<TCompCamera3rdPerson>();
      TCompCameraFlyover* c_cam2 = e_camera2->get<TCompCameraFlyover>();
      c_cam1->_enabled = false;
      c_cam2->_enabled = true;
      c_player->enabled = false;
    }
  }
}

void CModuleTestCameras::renderDebug()
{
}
