#include "mcv_platform.h"
#include "module_camera_shaker.h"
#include "entity/entity.h"
#include "engine.h"
#include "input/input.h"
#include "components/common/comp_transform.h"

std::mt19937 bt_cs(1279);
std::uniform_int_distribution<int> bt_rcs(0, 100);

bool CModuleCameraShaker::start()
{
  siv::PerlinNoise perlin(bt_rcs(bt_cs));
  siv::PerlinNoise perlin2(bt_rcs(bt_cs));
  siv::PerlinNoise perlin3(bt_rcs(bt_cs));

  perlin_noise.push_back(perlin);
  perlin_noise.push_back(perlin2);
  perlin_noise.push_back(perlin3);
  
  return true;
}

void CModuleCameraShaker::update(float delta) 
{
  PROFILE_FUNCTION("CModuleCameraShaker::update");

  delta = Time.delta_unscaled;

  if (EngineInput[VK_F3].justPressed()) {
    addTrauma(0.2f);
  }

  trauma = clamp(trauma - traumaDecreasingSpeed * delta, minTrauma, 1.f);
  shake = pow(trauma, 2.f);

  CEntity* e_cam = getEntityByName("PlayerCamera");
  TCompTransform* c_trans = e_cam->get<TCompTransform>();

  if (shake == 0.f) {//INTERPOLATE OFFSETS TO 0.f
    float speed = Interpolator::quadInOut(0.f, 1.f, timeToReturn / 0.5f);
    


    timeToReturn += delta;
  }
  else {
    timeToReturn = 0.f;
    //CALCULATE OFFSETS
    double p1 = perlin_noise.at(0).noise(Time.current * 3);
    double p2 = perlin_noise.at(1).noise(Time.current * 3);
    double p3 = perlin_noise.at(2).noise(Time.current * 3);

    yawOffset = maxYaw * shake * p1;
    pitchOffset = maxPitch * shake * p2;
    rollOffset = maxRoll * shake * p3;

    float yaw, pitch, roll;
    c_trans->getAngles(&yaw, &pitch, &roll);
    c_trans->setAngles(yawOffset, pitchOffset, rollOffset);
  }
}

void CModuleCameraShaker::stop() 
{

}

void CModuleCameraShaker::renderInMenu()
{
  if (ImGui::TreeNode("Camera Shake"))
  {
    ImGui::Text("Trauma:");
    ImGui::ProgressBar(trauma);
    ImGui::Text("Shake:");
    ImGui::ProgressBar(shake);

    ImGui::TreePop();
  }
}

void CModuleCameraShaker::renderDebug() 
{

}

void CModuleCameraShaker::addTrauma(float amount) 
{
  trauma = clamp(trauma + amount, minTrauma, 1.f);
}

void CModuleCameraShaker::setMinTrauma(float amount) 
{
  minTrauma = clamp(amount, 0.f, 1.f);
}