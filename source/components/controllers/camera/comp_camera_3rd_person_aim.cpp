#include "mcv_platform.h"
#include "comp_camera_3rd_person_aim.h"
#include "components/common/comp_transform.h"
#include "engine.h"
#include "input/input.h"
#include "modules/module_camera_mixer.h"
#include "components/common/comp_render.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/controllers/camera/comp_camera_3rd_person.h"
#include "comp_aim_controller.h"

DECL_OBJ_MANAGER("camera_3rd_person_aim", TCompCamera3rdPersonAim);

void TCompCamera3rdPersonAim::debugInMenu()
{
  ImGui::Text("Target: %.s", _target.isValid() ? ((CEntity*)_target)->getName() : "...");
  ImGui::Checkbox("Enabled", &_enabled);
  ImGui::SliderFloat3("Pos Offset", &_posOffset.x, -5.0f, 5.0f);
  ImGui::SliderFloat3("Target Offset", &_targetOffset.x, -5.0f, 5.0f);
  ImGui::SliderFloat("Yaw sensitivity", &_yawSensitivity, 0.0f, 5.0f);
  ImGui::SliderFloat("Pitch sensitivity", &_pitchSensitivity, 0.0f, 5.0f);
}

void TCompCamera3rdPersonAim::load(const json& j, TEntityParseContext& ctx) {
  _targetName = j.value("target", "");
  _targetOffset = loadVEC3(j, "targetOffset");
  _posOffset = loadVEC3(j, "posOffset");
  _ratio = j.value("ratio", _ratio);
  _curve = Resources.get(j.value("curve", ""))->as<CCurve>();
  _pitchSensitivity = j.value("pitchSensitivity", _pitchSensitivity);
  _yawSensitivity = j.value("yawSensitivity", _yawSensitivity);
}

bool TCompCamera3rdPersonAim::isPlayerMoving() {
  return EngineInput["front_"].value != 0.0f || EngineInput["back_"].value != 0.0f || EngineInput["left_"].value != 0.0f || EngineInput["right_"].value != 0.0f;
}

void TCompCamera3rdPersonAim::update(float scaled_dt)
{
	scaled_dt = Time.delta_unscaled;
  if (EngineInput[VK_F2].justPressed()) {
    mouse_active = !mouse_active;
  }

  if (!_target.isValid())
  {
    _target = getEntityByName(_targetName);

    if (!_target.isValid())
      return;
  }

  TCompTransform* cTransform = get<TCompTransform>();
  if (!cTransform)
    return;

  CEntity* eTarget = _target;
  TCompTransform* cTargetTransform = eTarget->get<TCompTransform>();
  if (!cTargetTransform)
    return;

  // treat input
  float pitch_rotation = 0.0f;
  float yaw_rotation = 0.0f;
  treatInput(yaw_rotation, pitch_rotation);

  // yaw update
  const float yawOffset = (yaw_rotation)* GameController.yaw_sensivity * scaled_dt;
  _yaw -= yawOffset;

  // pitch update
  const float ratioOffset = (pitch_rotation)* GameController.pitch_sensivity * scaled_dt;
  _ratio = clamp(_ratio + ratioOffset, 0.f, 0.99999f);

  // curve transform
  const MAT44 mRotation = MAT44::CreateFromYawPitchRoll(_yaw, 0.f, 0.f);
  const MAT44 mTranslation = MAT44::CreateTranslation(cTargetTransform->getPosition());
  _curveTransform = mRotation * mTranslation * MAT44::Identity;

  // final entity transform
  VEC3 posInCurve = _curve->evaluate(_ratio);
  posInCurve += _posOffset;
  VEC3 newPos = VEC3::Transform(posInCurve, _curveTransform);

  //treat target offset
  VEC3 targetPos = cTargetTransform->getPosition() + _targetOffset;

  const VEC3 actualPos = cTransform->getPosition();

  // smooth camera movement
  if (isPlayerMoving() || yaw_rotation != 0.f || pitch_rotation != 0.f) {
    timeMovingCamera += scaled_dt;
  }
  else {
    timeMovingCamera = 0.f;
  }

  interpolation = Interpolator::quadInOut(0.f, 1.f, timeMovingCamera / startEndTime);
  interpolation = clamp(interpolation, 0.7f, 1.f);
  //const VEC3 smoothedPosition = VEC3::Lerp(actualPos, newPos, smoothSpeed * scaled_dt); //LERP

  VEC3 smoothedPosition = actualPos + (newPos - actualPos) * 10.0f * scaled_dt * interpolation; //ASYMPTOTIC AVERAGE

  cTransform->lookAt(smoothedPosition, targetPos);

  shouldSwapCamera();
}

void TCompCamera3rdPersonAim::treatCollisionWithScenario(VEC3 &newPos, VEC3 &targetPos) {

  VEC3 rayDir = newPos - targetPos;
  float dist = abs(VEC3::Distance(newPos, targetPos));
  rayDir.Normalize();
  PxRaycastBuffer hit;
  PxRaycastHit hitBuffer[10];
  hit = PxRaycastBuffer(hitBuffer, 10);
  const PxHitFlags outputFlags =
    PxHitFlag::eDISTANCE
    | PxHitFlag::ePOSITION
    | PxHitFlag::eNORMAL
    ;

  PxQueryFilterData filter_data = PxQueryFilterData();
  filter_data.data.word0 = EnginePhysics.CameraCollision;

  //SCENE COLLISION NEEDS TO BE TESTED
  bool colDetected = EnginePhysics.gScene->raycast(VEC3_TO_PXVEC3(targetPos), VEC3_TO_PXVEC3(rayDir), dist, hit, outputFlags, filter_data);
  if (colDetected) {
    int closestIdx = -1;
    float closestDist = 1000.0f;
    for (int i = 0; i < hit.getNbAnyHits(); i++) {
      if (hit.getAnyHit(i).distance <= closestDist) {
        closestDist = hit.getAnyHit(i).distance;
        closestIdx = i;
      }
    }

    if (closestIdx != -1) {
      // separate a bit more the camera from the wall
      newPos = PXVEC3_TO_VEC3(hit.getAnyHit(closestIdx).position) + PXVEC3_TO_VEC3(hit.getAnyHit(closestIdx).normal) * 0.3f;
    }
  }

}

void TCompCamera3rdPersonAim::treatInput(float &yaw_rotation, float &pitch_rotation) {
  auto& mouse = EngineInput.mouse();
  VEC2 mOff = mouse.getDelta();

  if (_enabled && mouse_active) {
    if (mOff.x != 0.0f) { yaw_rotation = mOff.x; }
    if (mOff.y != 0.0f) { pitch_rotation = mOff.y; }
  }

  //INPUT FROM GAMEPAD
  if (EngineInput.gamepad()._connected && _enabled) { //IF CONNECTED, INPUT FROM GAMEPAD
    if (EngineInput["rleft_"].value != 0.0f) { yaw_rotation = EngineInput["rleft_"].value; }
    if (EngineInput["up_"].value != 0.0f) { pitch_rotation = -EngineInput["up_"].value; }
  }

  VEC2 input_dir = VEC2(yaw_rotation, pitch_rotation);
  float length = clamp(input_dir.Length(), 0.f, 1.f);
  input_dir.Normalize();

  length = Interpolator::cubicIn(0.f, 1.f, length);

  if (mOff != VEC2()) {
    if (abs(mOff.x) > 2.f)
      mOff.x = 2.f;
    if (abs(mOff.y) > 2.f)
      mOff.y = 2.f;
    yaw_rotation = input_dir.x * length * abs(mOff.x);
    pitch_rotation = input_dir.y * length * abs(mOff.y);
  }
  else {
    yaw_rotation = input_dir.x * length;
    pitch_rotation = input_dir.y * length;
  }
}

void TCompCamera3rdPersonAim::shouldSwapCamera() {

  //SWAP BETWEEN AIM AND NORMAL CAMERA
  if (EngineInput["aim_"].justReleased() && _enabled) {
    const float kBlendTime = 0.5f;
    static Interpolator::TCubicInOutInterpolator quadInt;
    CEntity* e_camera = getEntityByName("PlayerCamera");
    Engine.getCameraMixer().blendCamera(e_camera, kBlendTime, &quadInt);
    CEntity* ui = getEntityByName("Mirilla");
    TCompRender* c_render = ui->get<TCompRender>();
    c_render->is_visible = false;
    c_render->updateRenderManager();

    GameController.yaw_sensivity /= 0.4f;
    GameController.pitch_sensivity /= 0.4f;
  }
}

void TCompCamera3rdPersonAim::renderDebug()
{
  if (_curve)
  {
    CTransform tr;
    tr.fromMatrix(_curveTransform);
    _curve->renderDebug(tr);
  }
  if (curve_dynamic)
  {
	  CTransform tr;
	  tr.fromMatrix(MAT44::Identity);
	  curve_dynamic->renderDebug(tr);
  }
}

void TCompCamera3rdPersonAim::setTimeScale(float timeScale) {
  assert(timeScale >= 0.0);
  _timeScale = timeScale;
}

float TCompCamera3rdPersonAim::getTimeScale() {
  return _timeScale;
}