#include "mcv_platform.h"
#include "comp_camera_flyover.h"
#include "components/common/comp_transform.h"
#include "utils/utils.h"
#include "engine.h"
#include "input/input.h"
#include "input/module_input.h"
#include "modules/module_camera_mixer.h"


DECL_OBJ_MANAGER("camera_flyover", TCompCameraFlyover);

void TCompCameraFlyover::debugInMenu() {
  ImGui::Checkbox("Enabled", &_enabled);
  ImGui::DragFloat("Max Speed", &_speed, 0.1f, 1.f, 100.f);
  ImGui::DragFloat("Sensitivity", &_sensitivity, 0.001f, 0.001f, 0.1f);
  ImGui::DragFloat("Inertia", &_ispeed_reduction_factor, 0.001f, 0.7f, 1.f);
  ImGui::LabelText("Curr Speed", "%f", _ispeed);
}

void TCompCameraFlyover::load(const json& j, TEntityParseContext& ctx) {
  _speed = j.value("speed", _speed);
  _sensitivity = j.value("sensitivity", _sensitivity);
  _ispeed_reduction_factor = j.value("speed_reduction_factor", _ispeed_reduction_factor);
  _enabled = j.value("enabled", _enabled);
  if (j.count("key")) {
    std::string k = j["key"];
    _key_toggle_enabled = k[0];
  }
}

void TCompCameraFlyover::update(float scaled_dt)
{

  if (!_enabled)
    return;

  // Because we are moving the camera...
  const auto& but_right = EngineInput[Input::BT_MOUSE_RIGHT];
  if (!but_right.isPressed())
    return;

  TCompTransform* c_transform = get<TCompTransform>();
  if (!c_transform)
    return;

  // if the game is paused, we still want full camera speed 
  float dt = Time.delta_unscaled;

  VEC3 pos = c_transform->getPosition();
  VEC3 front = c_transform->getFront();
  VEC3 left = c_transform->getLeft();
  VEC3 up = VEC3::UnitY;

  _ispeed *= _ispeed_reduction_factor;

  // movement
  float deltaSpeed = _speed * dt;
  //if (EngineInput["turbo"])
  if (isPressed(VK_SHIFT))
    deltaSpeed *= 4.f;

  //if (EngineInput["front"].value)
  if (EngineInput["front_"].isPressed())
    _ispeed.z = 1.f;
  //if (isPressed('S'))
  if (EngineInput["back_"].isPressed())
    _ispeed.z = -1.f;
  //if (isPressed('A'))
  if (EngineInput["left_"].isPressed())
    _ispeed.x = 1.f;
  //if (isPressed('D'))
  if (EngineInput["right_"].isPressed())
    _ispeed.x = -1.f;
  if (isPressed('R'))
    //if (EngineInput["up"].value)
    _ispeed.y = 1.f;
  if (isPressed('F'))
    //if (EngineInput["down"].value)
    _ispeed.y = -1.f;

  // Amount in each direction
  VEC3 off;
  off += front * _ispeed.z * deltaSpeed;
  off += left * _ispeed.x * deltaSpeed;
  off += up * _ispeed.y * deltaSpeed;

  // rotation
  float yaw, pitch;
  vectorToYawPitch(front, &yaw, &pitch);

  VEC2 mOff = EngineInput.mouse().getDelta();
  yaw += -mOff.x * _sensitivity;
  pitch += mOff.y * _sensitivity;
  if (pitch < -_maxPitch)
    pitch = -_maxPitch;
  else if (pitch > _maxPitch)
    pitch = _maxPitch;

  // final values
  VEC3 newPos = pos + off;
  VEC3 newFront = yawPitchToVector(yaw, pitch);

  c_transform->lookAt(newPos, newPos + newFront);
}
