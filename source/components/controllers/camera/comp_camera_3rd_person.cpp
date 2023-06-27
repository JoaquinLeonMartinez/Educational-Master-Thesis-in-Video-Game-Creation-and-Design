#include "mcv_platform.h"
#include "comp_camera_3rd_person.h"
#include "components/common/comp_transform.h"
#include "engine.h"
#include "input/input.h"
#include "modules/module_camera_mixer.h"
#include "components/common/comp_render.h"
#include "components/controllers/character/comp_character_controller.h"
#include "comp_aim_controller.h"
#include "components/controllers/comp_parabolic_launch.h"
#include "components/common/comp_camera.h"
#include "geometry/interpolators.h"
#include "ui/widgets/ui_image.h"
#include "ui/module_ui.h"

using namespace physx;

DECL_OBJ_MANAGER("camera_3rd_person", TCompCamera3rdPerson);

void TCompCamera3rdPerson::debugInMenu()
{
  ImGui::Text("Target: %.s", _target.isValid() ? ((CEntity*)_target)->getName() : "...");
  ImGui::Checkbox("Enabled", &_enabled);
  ImGui::Checkbox("Aiming", &aiming);
  ImGui::SliderFloat("Camera Aim Transition", &interpolation, 0.0f, 1.0f);
  ImGui::SliderFloat("Smooth Speed", &smoothSpeed, 10.0f, 20.0f);
  ImGui::SliderFloat("Yaw Sensitivity", &GameController.yaw_sensivity, 10.0f, 20.0f);
  ImGui::SliderFloat("Pitch Sensitivity", &GameController.pitch_sensivity, 10.0f, 20.0f);
  ImGui::DragFloat3("Pos Offset", &_posOffset.x, 0.01f, 0.0f, 25.0f);
  ImGui::DragFloat3("Aim Offset", &aimOffset.x, 0.01f, 0.0f, 25.0f);


}

void TCompCamera3rdPerson::load(const json& j, TEntityParseContext& ctx) {
  _targetName = j.value("target", "");
  _posOffset = loadVEC3(j, "posOffset");
  _ratio = j.value("ratio", _ratio);
  _curve = Resources.get(j.value("curve", ""))->as<CCurve>();
  _curveIdle = Resources.get(j.value("curveIdle", ""))->as<CCurve>();
 
}

bool TCompCamera3rdPerson::isPlayerMoving() {
  return EngineInput["front_"].value != 0.0f || EngineInput["back_"].value != 0.0f || EngineInput["left_"].value != 0.0f || EngineInput["right_"].value != 0.0f;
}

bool TCompCamera3rdPerson::isCameraRotating() {
  auto& mouse = EngineInput.mouse();
  VEC2 mOff = mouse.getDelta();
  return EngineInput["rleft_"].value != 0.0f || EngineInput["up_"].value != 0.0f || mOff.x != 0.0f || mOff.y != 0.0f;
}

void TCompCamera3rdPerson::resetCamera() {

  if (!Time.real_scale_factor)
    return;

  TCompTransform* cTransform = get<TCompTransform>();
  if (!cTransform)
    return;

  CEntity* eTarget = _target;
  if (!eTarget)
    return;
  TCompTransform* cTargetTransform = eTarget->get<TCompTransform>();
  if (!cTargetTransform)
    return;

  
  float player_pitch_rotation = 0.0f;
  float player_yaw_rotation = 0.0f;
  cTargetTransform->getAngles(&player_yaw_rotation, &player_pitch_rotation);
  _yaw = player_yaw_rotation;
  _ratio = 0.5f;

  // curve transform movement
  const MAT44 mRotation = MAT44::CreateFromYawPitchRoll(_yaw, 0.f, 0.f);
  VEC3 playerPos = cTargetTransform->getPosition();

  const MAT44 mTranslation = MAT44::CreateTranslation(playerPos);
  _curveTransform = mRotation * mTranslation * MAT44::Identity;

  // final entity transform
  VEC3 posInCurve = _curve->evaluate(0.f);

  //treat if is aiming or not
  actualAimTransitionTime = clamp(actualAimTransitionTime, 0.f, aimTransitionTime);
  interpolation = Interpolator::quadInOut(0.f, 1.f, actualAimTransitionTime / aimTransitionTime);
  posInCurve = posInCurve + _posOffset + aimOffset * interpolation;
  VEC3 aimPosInCurve = posInCurve + aimOffset * (1.0f - interpolation);

  VEC3 newPos = VEC3::Transform(posInCurve, _curveTransform);
  VEC3 aimPos = VEC3::Transform(aimPosInCurve, _curveTransform);

  //idle calculations
  const MAT44 mIdleRotation = MAT44::CreateFromYawPitchRoll(_yaw, 0.f, 0.f); //should be also pitch?
  const MAT44 mIdleTranslation = MAT44::CreateTranslation(newPos);
  _curvePulseTransform = mIdleRotation * mIdleTranslation * MAT44::Identity;

  VEC3 pulsePosInCurve = _curveIdle->evaluate(0.f);

  pulsePos = VEC3::Transform(pulsePosInCurve, _curvePulseTransform);

  VEC3 semiFinalPos = pulsePos;
  putPlayerOnScreen(semiFinalPos);
  VEC3 finalPos = semiFinalPos;

  //target position to look at
  VEC3 dir = playerPos + VEC3(0.f, 2.1f, 0.f) - aimPos;
  dir.Normalize();
  VEC3 targetPos = finalPos + dir * distance;

  cTransform->lookAt(finalPos, targetPos);

  first_reset = true;
}

void TCompCamera3rdPerson::update(float scaled_dt)
{
  if (!first_reset) // so the camera starts with the player
    resetCamera();
    scaled_dt = Time.delta_unscaled;
  if (scaled_dt >= 0.03333f) { //less than 30 frames per second it will be loading
    scaled_dt = 0.03333f; //update it as if it was moving normally
  }

	if(!isPause){

	  if (EngineInput[VK_F2].justPressed()) {
		  mouse_active = !mouse_active;
	  }

	  if (!_target.isValid())
	  {
		  _target = getEntityByName(_targetName);

		  if (!_target.isValid())
		    return;
	  }

	  if (EngineInput["reset_camera_"].justPressed()) {
		  resetCamera();
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

	  float newYawOffset = (yaw_rotation)* GameController.yaw_sensivity * scaled_dt;
	  //smooth camera rotation
	  yawOffset = Damp(yawOffset * Time.real_scale_factor, newYawOffset, smoothSpeed, scaled_dt);
	  // yaw update
	  _yaw -= yawOffset;

	  // pitch update
	  float newRatioOffset = (pitch_rotation) * GameController.pitch_sensivity * scaled_dt;
	  ratioOffset = Damp(ratioOffset * Time.real_scale_factor, newRatioOffset, smoothSpeed, scaled_dt);
	  _ratio = clamp(_ratio + ratioOffset, 0.f, 0.99999f);

	  // curve transform movement
	  //float cameraHeight = cameraMovementOnJump();
	  const MAT44 mRotation = MAT44::CreateFromYawPitchRoll(_yaw, 0.f, 0.f);
	  VEC3 playerPos = cTargetTransform->getPosition();
	  /*if (!actualCameraHeight) {//first time only
		actualCameraHeight = cameraHeight;
		playerPos.y = cameraHeight;
	  }
	  else {
		actualCameraHeight = Damp(actualCameraHeight, cameraHeight, smoothSpeed, scaled_dt);
		//actualCameraHeight += (cameraHeight - actualCameraHeight) * smoothSpeed * scaled_dt; //ASYMPTOTIC AVERAGE ON CAMERA JUMP
		playerPos.y = actualCameraHeight;
	  }*/
  
	  const VEC3 smoothedPos = Damp(_curveTransform.Translation(), playerPos, smoothSpeed, scaled_dt);
	  const MAT44 mTranslation = MAT44::CreateTranslation(smoothedPos);
	  _curveTransform = mRotation * mTranslation * MAT44::Identity;

	  // final entity transform
	  VEC3 posInCurve = _curve->evaluate(_ratio);

	  //treat if is aiming or not
	  if (aiming) {
		  actualAimTransitionTime += scaled_dt;
	  }
	  else {
		  actualAimTransitionTime -= scaled_dt;
	  }
	  actualAimTransitionTime = clamp(actualAimTransitionTime, 0.f, aimTransitionTime);
	  interpolation = Interpolator::quadInOut(0.f, 1.f, actualAimTransitionTime / aimTransitionTime);
	  posInCurve = posInCurve + _posOffset + aimOffset * interpolation;
	  VEC3 aimPosInCurve = posInCurve + aimOffset * (1.0f - interpolation);

	  VEC3 newPos = VEC3::Transform(posInCurve, _curveTransform);
	  VEC3 aimPos = VEC3::Transform(aimPosInCurve, _curveTransform);

	  //idle calculations
	  const MAT44 mIdleRotation = MAT44::CreateFromYawPitchRoll(_yaw, 0.f, 0.f);
	  const MAT44 mIdleTranslation = MAT44::CreateTranslation(newPos);
	  _curvePulseTransform = mIdleRotation * mIdleTranslation * MAT44::Identity;

	  VEC3 pulsePosInCurve = _curveIdle->evaluate(pulseRatio);

	  pulsePos = VEC3::Transform(pulsePosInCurve, _curvePulseTransform);

	  //if not moving, move camera around pulse curve
	  if (!isPlayerMoving() && !isCameraRotating()) {
		  pulseRatio = clamp(pulseRatio + scaled_dt * 0.05f, 0.f, 0.99999f);
		  if (pulseRatio == 0.99999f)
  		  pulseRatio = 0.f;
	  }
  
	  VEC3 semiFinalPos = pulsePos;
	  putPlayerOnScreen(semiFinalPos);
	  VEC3 finalPos;
	  //dbg("----------\n");
	  if (semiFinalPos == pulsePos) { //NO COLLISION
		//dbg("No collision\n");
		 if (lastPosWasCollision) { //IF LAST POSITION WAS COLLISION OR TRANSITION FROM COLLISION 
		  // dbg("Transition from collision at %f %f %f  to  %f %f %f \n",lastPos.x, lastPos.y,lastPos.z, semiFinalPos.x, semiFinalPos.y, semiFinalPos.z);

		   //finalPos = lastPos + (semiFinalPos - lastPos) * smoothSpeed * 8.0f * scaled_dt; //ASYMPTOTIC AVERAGE
		   finalPos = Damp(lastPos, semiFinalPos, smoothSpeed * 8.0f, scaled_dt);
		   if (abs(VEC3::Distance(finalPos, lastPos)) < 0.005f) {//IF THIS NEW POSITION IS THE END OF TRANSITION
			 //dbg("End transition from collision\n");
			 lastPosWasCollision = false;
		   }
		 }
		 else {
		   //dbg("No transition from collision\n");
		   finalPos = semiFinalPos;
		   lastPosWasCollision = false;
		 }
	  }
	  else { //COLLIDES, ASYMPTOTIC AVERAGE ON COLLISION MOVEMENT
		lastPosWasCollision = true;
		//finalPos = lastPos + (semiFinalPos - lastPos) * smoothSpeed * 4.0f * scaled_dt; //ASYMPTOTIC AVERAGE
		finalPos = Damp(lastPos, semiFinalPos, smoothSpeed * 4.0f, scaled_dt);
		//dbg("Collision at %f %f %f\n", finalPos.x, finalPos.y, finalPos.z);
	  }
	  lastPos = finalPos;
  
	  //target position to look at
	  VEC3 dir = smoothedPos + VEC3(0.f, 2.1f, 0.f) - aimPos;
	  dir.Normalize();
	  VEC3 targetPos = finalPos + dir * distance;

	  cTransform->lookAt(finalPos, targetPos);

    shouldSwapCamera();
	 }


}

float TCompCamera3rdPerson::cameraMovementOnJump() {
  CEntity* eTarget = _target;
  TCompTransform* c_trans = eTarget->get<TCompTransform>();
  VEC3 playerPos = c_trans->getPosition();

  VEC3 rayDir = VEC3(0.f,-1.f, 0.f);
  float dist = 1.f;
  PxRaycastBuffer hit;
  PxRaycastHit hitBuffer[10];
  hit = PxRaycastBuffer(hitBuffer, 10);
  const PxHitFlags outputFlags =
    PxHitFlag::eDISTANCE
    | PxHitFlag::ePOSITION
    | PxHitFlag::eNORMAL
    ;

  PxQueryFilterData filter_data = PxQueryFilterData();
  filter_data.data.word0 = EnginePhysics.CameraJump;

  //SCENE COLLISION NEEDS TO BE TESTED
  bool colDetected = EnginePhysics.gScene->raycast(VEC3_TO_PXVEC3(playerPos), VEC3_TO_PXVEC3(rayDir), dist, hit, outputFlags, filter_data);
  int closestIdx = -1;
  if (colDetected) {
    float closestDist = 1000.0f;
    for (int i = 0; i < hit.getNbAnyHits(); i++) {
      if (hit.getAnyHit(i).distance <= closestDist) {
        closestDist = hit.getAnyHit(i).distance;
        closestIdx = i;
      }
    }

    if (closestIdx != -1) {
      // separate a bit more the camera from the wall
      CHandle h_collision;
      h_collision.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
      if (h_collision.isValid()) {
        lastGroundHandle = h_collision;
        lastHeight = hit.getAnyHit(closestIdx).position.y;
        return lastHeight;
      }
      
    }
  }

  return lastHeight;
}

bool TCompCamera3rdPerson::putPlayerOnScreen(VEC3 &newPos) {
  CEntity* eTarget = _target;
  TCompTransform* c_trans = eTarget->get<TCompTransform>();
  VEC3 playerPos = c_trans->getPosition();
  playerPos.y += 1.8f;

  VEC3 rayDir = newPos - playerPos;
  float dist = abs(VEC3::Distance(newPos, playerPos));
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
  /*if (EnginePhysics.gScene == nullptr) {
	  return false;
  }*/
  bool colDetected = EnginePhysics.gScene->raycast(VEC3_TO_PXVEC3(playerPos), VEC3_TO_PXVEC3(rayDir), dist, hit, outputFlags, filter_data);
  int closestIdx = -1;
  if (colDetected) {
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
      return true;
    }
  }
  
  return false;
}

void TCompCamera3rdPerson::treatInput(float &yaw_rotation, float &pitch_rotation) {
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

void TCompCamera3rdPerson::shouldSwapCamera() {

  if (!_enabled)
    return;

  //SWAP BETWEEN AIM AND NORMAL CAMERA
  if (EngineInput["aim_"].isPressed()) {
    if (!aiming) {
      UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
      mirilla->getParams()->visible = true;
    }
    aiming = true;
    smoothSpeed = 10.f;
    GameController.yaw_sensivity = 2.0f;
    GameController.pitch_sensivity = 0.8f;
  }

  //SWAP BETWEEN AIM AND NORMAL CAMERA
  if (!EngineInput["aim_"].isPressed()) {
    if (aiming) {
      UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
      mirilla->getParams()->visible = false;
    }
    aiming = false;
    smoothSpeed = 6.f;
    GameController.yaw_sensivity = 5.0f;
    GameController.pitch_sensivity = 2.0f;

  }
}

void TCompCamera3rdPerson::renderDebug()
{
  if (_curve)
  {
    CTransform tr;
    tr.fromMatrix(_curveTransform);
    _curve->renderDebug(tr);
  }

  if (_curveIdle)
  {
    CTransform tr;
    tr.fromMatrix(_curvePulseTransform);
    _curveIdle->renderDebug(tr);
  }
}

void TCompCamera3rdPerson::registerMsgs() {
	DECL_MSG(TCompCamera3rdPerson, TMsgGamePause, onPauseCam);
}

void TCompCamera3rdPerson::onPauseCam(const TMsgGamePause &msg) {
	isPause = msg.isPause;
}


