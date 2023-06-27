#include "mcv_platform.h"
#include "comp_player_controller.h"
#include "components/common/comp_transform.h"
#include "engine.h"
#include "input/input.h"
#include "input/enums.h"
#include "modules/module_physics.h"

using namespace physx;

DECL_OBJ_MANAGER("player_controller", TCompPlayerController);

void TCompPlayerController::Init() {
	AddState("GROUNDED", (statehandler)&TCompPlayerController::grounded);
	AddState("DASHING", (statehandler)&TCompPlayerController::dashing);
	AddState("ON_AIR", (statehandler)&TCompPlayerController::onAir);
	AddState("HOVERING", (statehandler)&TCompPlayerController::hovering);

	//ADD MORE STATES FOR BEING HIT, ETC, ETC

	ChangeState("GROUNDED");
}

void TCompPlayerController::load(const json& j, TEntityParseContext& ctx) {
	this->Init();
	speed = j.value("speed", speed);
	rotation_speed = j.value("rotation_speed", rotation_speed);
}

void TCompPlayerController::debugInMenu() {
  ImGui::LabelText("State", "%s", state.c_str());
  ImGui::SliderFloat3("Impulse", &impulse.x, -5.0f, 5.0f);
  ImGui::DragFloat("Jump Force", &jump_force, 0.5f, 0.f, 30.f);
  ImGui::DragFloat("Double Jump Force", &double_jump_force, 0.5f, 0.f, 30.f);
  ImGui::DragFloat("Falling factor on hover", &falling_factor_hover, 0.1f, 0.f, 1.f);
  ImGui::DragFloat("Moving factor on hover", &moving_factor_hover, 0.1f, 0.f, 1.f);
  ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 10.f);
  ImGui::DragFloat("Dash Speed", &dash_speed, 0.1f, 0.f, 10.f);
  ImGui::DragFloat("Rotation speed", &rotation_speed, 0.1f, 0.f, 10.f);
} 

//TODO: BUG OF ISGROUNDED

void TCompPlayerController::grounded(float delta) {
	//WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
	if (!h_camera.isValid()) {
		h_camera = getEntityByName("Camera");
		return;
	}

	treatRumble(delta);
	bool startDash = false;
  can_double_jump = true;

	VEC3 dir = VEC3(0.0f, -9.81f, 0.0f);

	//GROUNDED, PLAYER CAN AIM, SHOOT, DASH, JUMP AND MOVE
	weaponSelection();
	aiming = false;

  //MOVEMENT
  getInputForce(dir);

	if (EngineInput["aim_"].isPressed()) {//AIM
		aiming = true;
	}
	if (EngineInput["shoot_"].justPressed()) {//SHOOT
		shoot();
	}
	if (EngineInput["dash_"].justPressed()) {//DASH
		ChangeState("DASHING");
		dash = dash_limit;
		startDash = true;
	}
	else if (EngineInput["jump_"].justPressed()) {//JUMP
    impulse = dir;
    impulse.y = jump_force;
		ChangeState("ON_AIR");
  }
  else if (!isGrounded()) { //FALLING
    impulse = dir;
    ChangeState("ON_AIR");
  }
	else if (EngineInput["attack_"].justPressed()) {//ATTACK
		attack();
	}

  dir *= delta;

	//MOVE PLAYER
	TCompCharacterCollider* c_cc = get<TCompCharacterCollider>();
	if(c_cc)
		c_cc->gController->move(VEC3_TO_PXVEC3(dir), 0.0f, delta, PxControllerFilters());

	//WHEN CHARACTER IS GROUNDED HE CAN ROTATE
	rotatePlayer(dir, delta, startDash);
}

void TCompPlayerController::dashing(float delta) {
	//WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
	if (!h_camera.isValid()) {
		h_camera = getEntityByName("Camera");
		return;
	}

	treatRumble(delta);

	//IMPORTANT, DASHING WE DONT APPLY GRAVITY, FOR SAKE OF AIR DASHING
	VEC3 dir = VEC3();

	//WHILE DASHING, PLAYER CANT AIM, MOVE AND SHOOT
	aiming = false;
	weaponSelection();

	//MOVEMENT
	TCompTransform* c_trans = get<TCompTransform>();
	dir = c_trans->getFront() * dash_speed;
  impulse = dir * 0.7f;
	dir *= delta;

	//MOVE PLAYER
	TCompCharacterCollider* c_cc = get<TCompCharacterCollider>();
	if (c_cc)
		c_cc->gController->move(VEC3_TO_PXVEC3(dir), 0.0f, delta, PxControllerFilters());

	//WHEN DASHING, PLAYER CANT ROTATE

	dash -= delta;
	if (dash <= 0.0f) {
		//DEPENDING ON RAYCAST WILL GO TO ON_AIR OR GROUNDED
		if (isGrounded()) {
			ChangeState("GROUNDED");
      impulse = VEC3();
		}
		else {
			ChangeState("ON_AIR");
		}
	}


}

void TCompPlayerController::onAir(float delta) {
	//WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
	if (!h_camera.isValid()) {
		h_camera = getEntityByName("Camera");
		return;
	}

  if (isGrounded() && impulse.y <= 0.0f) {
		ChangeState("GROUNDED");
    impulse = VEC3();
		return;
	}

	treatRumble(delta);
	bool startDash = false;

	VEC3 dir = VEC3(0.0f, -9.81f, 0.0f);

	//ON AIR, PLAYER CAN AIM, SHOOT AND DASH, HE CANT MOVE
	aiming = false;
	weaponSelection();
	if (EngineInput["aim_"].isPressed()) {//AIM
		aiming = true;
	}
	if (EngineInput["shoot_"].justPressed() && aiming) {//SHOOT
		shoot();
	}
	if (EngineInput["dash_"].justPressed()) {//DASH
		ChangeState("DASHING");
		dash = dash_limit;
		startDash = true;
    impulse = VEC3();
	}
	else if (EngineInput["jump_"].justPressed() && can_double_jump) { //DOUBLE JUMP
    can_double_jump = false;
    VEC3 new_impulse = VEC3();
    getInputForce(new_impulse);
    impulse = new_impulse;
    impulse *= 0.7;
    impulse.y = double_jump_force;
  }
  else if (EngineInput["jump_"].isPressed() && !can_double_jump && impulse.y <= 0.0f) { //HOVERING
    ChangeState("HOVERING");
    impulse = VEC3();
  }

  //GRAVITY
  dir = impulse;
	dir *= delta;
  impulse.y -= 9.81f * delta;

	//MOVE PLAYER TO THE GROUND
	TCompCharacterCollider* c_cc = get<TCompCharacterCollider>();
	if (c_cc)
		c_cc->gController->move(VEC3_TO_PXVEC3(dir), 0.0f, delta, PxControllerFilters());

  //WE GET THE INPUT FORCE SO WE CAN ROTATE THE PLAYER TO THAT DIRECTION, NOT MOVE!
  getInputForce(dir);

	//ON AIR, THE CHARACTER CAN MOVE OR ROTATE
	rotatePlayer(dir, delta, startDash);
}

void TCompPlayerController::hovering(float delta) {
	//WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
	if (!h_camera.isValid()) {
		h_camera = getEntityByName("Camera");
		return;
	}

	treatRumble(delta);
	bool startDash = false;

  if (isGrounded() && impulse.y <= 0.0f) {
    ChangeState("GROUNDED");
    impulse = VEC3();
    return;
  }
  else if (EngineInput["jump_"].justReleased()) {
    ChangeState("ON_AIR");
    getInputForce(impulse);
    impulse *= moving_factor_hover;
    impulse.y = -9.81f * falling_factor_hover;
    return;
  }

	VEC3 dir = VEC3(0.0f, -9.81f, 0.0f);

	//WHILE HOVERING, PLAYER CAN AIM, SHOOT, DASH AND MOVE
	aiming = false;
	weaponSelection();

	if (EngineInput["aim_"].isPressed()) {//AIM
		aiming = true;
	}
	if (EngineInput["shoot_"].justPressed() && aiming) {//SHOOT
		shoot();
	}
	if (EngineInput["dash_"].justPressed()) {//DASH
		ChangeState("DASHING");
		dash = dash_limit;
		startDash = true;
	}
	//MOVEMENT
	getInputForce(dir);
	dir *= delta;
	//REDUCE PLAYER SPEED WHILE ON AIR
	dir.x *= moving_factor_hover;
  dir.z *= moving_factor_hover;
  dir.y *= falling_factor_hover;
	
	//MOVE PLAYER
	TCompCharacterCollider* c_cc = get<TCompCharacterCollider>();
	if (c_cc)
		c_cc->gController->move(VEC3_TO_PXVEC3(dir), 0.0f, delta, PxControllerFilters());

	//WHEN CHARACTER IS HOVERING IT CAN BE ROTATED AND MOVED
	rotatePlayer(dir, delta, startDash);
}


void TCompPlayerController::treatRumble(float delta) {
	rumble_time -= delta;
	if (rumble_time < 0.0f) {
		Input::TRumbleData rumble;
		rumble.leftVibration = 0.0f;
		rumble.rightVibration = 0.0f;
		EngineInput.feedback(rumble);
		rumble_time = 0.0f;
	}
}

void TCompPlayerController::shoot() {
	if (EngineInput.gamepad()._connected) { //RUMBLE AT SHOOT
		Input::TRumbleData rumble;
		rumble.leftVibration = 0.2f;
		rumble.rightVibration = 0.2f;
		EngineInput.feedback(rumble);
		rumble_time = 0.3f;
	}

	TCompTransform* c_trans = get<TCompTransform>();

	if (weapon_selected == 0) {
		//characer Controller a de pegar con una fuerza;Necesito Character Controller
	}
	else if (weapon_selected == 1) {
		TEntityParseContext ctx;
		VEC3 posicion = VEC3(c_trans->getPosition().x, 1, c_trans->getPosition().z);
		c_trans->setPosition(posicion);
		ctx.root_transform = *c_trans;
		parseScene("data/prefabs/bullet_bounce.json", ctx); //cambiar prefab

		TMsgAssignBulletOwner msg;
		msg.h_owner = CHandle(this).getOwner();
		msg.source = c_trans->getPosition();
		msg.front = c_trans->getFront();
		ctx.entities_loaded[0].sendMsg(msg);
	}
	else if (weapon_selected == 2) {
		TEntityParseContext ctx;
		VEC3 posicion = VEC3(c_trans->getPosition().x, 1, c_trans->getPosition().z);
		c_trans->setPosition(posicion);
		ctx.root_transform = *c_trans;
		parseScene("data/prefabs/grenade.json", ctx); //cambiar prefab

		TMsgAssignGrenadeOwner msg;
		msg.h_owner = CHandle(this).getOwner();
		msg.source = c_trans->getPosition();
		msg.front = c_trans->getFront();
		ctx.entities_loaded[0].sendMsg(msg);
	}
}

void TCompPlayerController::attack() {

	
	TCompTransform* c_trans = get<TCompTransform>();
	TEntityParseContext ctx;
	//VEC3 posicion = VEC3(c_trans->getPosition().x,c_trans->getPosition().y, c_trans->getPosition().z);
	//c_trans->setPosition(posicion);
	ctx.root_transform = *c_trans;
	

	parseScene("data/prefabs/mop.json", ctx); //cambiar prefab

	TMsgAssignMopOwner msg;
	msg.h_owner = CHandle(this).getOwner();
	msg.source = c_trans->getPosition();
	msg.front = c_trans->getFront();
	ctx.entities_loaded[0].sendMsg(msg);

	
}

void TCompPlayerController::getInputForce(VEC3 &dir) {

	//Movement from camera perspective
	CEntity* e_camera = (CEntity *)h_camera;
	TCompTransform* cam_trans = e_camera->get<TCompTransform>();

	VEC3 camera_left = cam_trans->getLeft();
	camera_left.y = 0.0f;
	VEC3 camera_front = cam_trans->getFront();
	camera_front.y = 0.0f;

	//KEYBOARD
	if (EngineInput["front_"].isPressed()) {
		dir += camera_front * speed;
	}
	if (EngineInput["back_"].isPressed()) {
		dir -= camera_front * speed;
	}
	if (EngineInput["left_"].isPressed()) {
		dir += camera_left * speed;
	}
	if (EngineInput["right_"].isPressed()) {
		dir -= camera_left * speed;
	}
	//GAMEPAD
	if (EngineInput.gamepad()._connected) {
		dir += camera_front * EngineInput["front_"].value * speed;
		dir -= camera_left * EngineInput["left_"].value * speed;
	}
}

void TCompPlayerController::rotatePlayer(const VEC3 &dir, float delta, bool immediate) {
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 player_pos = c_trans->getPosition();

	CEntity* e_camera = (CEntity *)h_camera;
	TCompTransform* cam_trans = e_camera->get<TCompTransform>();
	VEC3 camera_front = cam_trans->getFront();
	camera_front.y = 0.0f;

	VEC3 norm_dir = VEC3();
	dir.Normalize(norm_dir);

	float elapsed = 1.0f;

	if ((dir.x != 0.0f || dir.z != 0.0f) && !aiming) { //ROTATE PLAYER WHERE HE WALKS
		//And also rotate the player on the direction its facing
		float yaw, pitch;
		c_trans->getAngles(&yaw, &pitch);
		
		float wanted_yaw = c_trans->getDeltaYawToAimTo(player_pos + dir);

		if (immediate) 
			elapsed = delta * rotation_speed;

		if (abs(wanted_yaw) > 0.01)
			c_trans->setRotation(QUAT::CreateFromYawPitchRoll(yaw + wanted_yaw * elapsed, pitch, 0.0f));
	}
	else if (aiming) { //ROTATE PLAYER WHERE THE CAMERA LOOKS
		float yaw, pitch;
		c_trans->getAngles(&yaw, &pitch);

		float wanted_yaw = c_trans->getDeltaYawToAimTo(player_pos + camera_front);

		if (immediate)
			elapsed = delta * rotation_speed * 2.0f;

		if (abs(wanted_yaw) > 0.01)
			c_trans->setRotation(QUAT::CreateFromYawPitchRoll(yaw + wanted_yaw * elapsed, pitch, 0.0f));
	}
}

bool TCompPlayerController::isGrounded() {
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 player_pos = c_trans->getPosition();
  player_pos.y += 1.0f;
	PxVec3 origin = VEC3_TO_PXVEC3(player_pos);
	PxVec3 unitDir = VEC3_TO_PXVEC3(VEC3(0, -1, 0));
	PxReal maxDistance = 0.2f;
	PxRaycastBuffer hit;

  bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit);
	return res;
}

void TCompPlayerController::weaponSelection() {
	if (EngineInput["select_banana_"].justPressed()) { //lanza platanos 
		weapon_selected = 1;
	}
	else if (EngineInput["select_grenade_"].justPressed()) { //guindilla
		weapon_selected = 2;
	}
}

/*void TCompPlayerController::update(float delta) {
	if (!h_camera.isValid()) {
		h_camera = getEntityByName("Camera");
		return;
	}

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 player_pos = c_trans->getPosition();

	//Movement from camera perspective
	CEntity* e_camera = (CEntity *)h_camera;
	TCompTransform* cam_trans = e_camera->get<TCompTransform>();
	VEC3 camera_left = cam_trans->getLeft();
	camera_left.y = 0.0f;

	VEC3 camera_front = cam_trans->getFront();
	camera_front.y = 0.0f;

	VEC3 dir = VEC3();

	//KEYBOARD
	if (EngineInput["up"].isPressed()) {
		player_pos = player_pos + camera_front * speed * delta;
		dir += camera_front;
	}
	if (EngineInput["down"].isPressed()) {
		player_pos = player_pos - camera_front * speed * delta;
		dir -= camera_front;
	}
	if (EngineInput["left"].isPressed()) {
		player_pos = player_pos + camera_left * speed * delta;
		dir += camera_left;
	}
	if (EngineInput["right"].isPressed()) {
		player_pos = player_pos - camera_left * speed * delta;
		dir -= camera_left;
	}

	rumble_time -= delta;
	if (rumble_time < 0.0f) {
		Input::TRumbleData rumble;
		rumble.leftVibration = 0.0f;
		rumble.rightVibration = 0.0f;
		EngineInput.feedback(rumble);
		rumble_time = 0.0f;
	}

	aiming = false;
	if (EngineInput["aim_"].isPressed()) {
		aiming = true;
	}
	if (EngineInput["shoot_"].justPressed() && aiming) {//SHOOT
		if (EngineInput.gamepad()._connected) { //RUMBLE AT SHOOT
			Input::TRumbleData rumble;
			rumble.leftVibration = 0.2f;
			rumble.rightVibration = 0.2f;
			EngineInput.feedback(rumble);
			rumble_time = 0.3f;
		}
		if (estadoMockControl == 0) {
			//characer Controller a de pegar con una fuerza;Necesito Character Controller
		}
		else if (estadoMockControl == 1) {
			TEntityParseContext ctx;
			VEC3 posicion = VEC3(c_trans->getPosition().x, 1, c_trans->getPosition().z);
			c_trans->setPosition(posicion);
			ctx.root_transform = *c_trans;
			parseScene("data/prefabs/bullet_bounce.json", ctx); //cambiar prefab

			TMsgAssignBulletOwner msg;
			msg.h_owner = CHandle(this).getOwner();
			msg.source = c_trans->getPosition();
			msg.front = c_trans->getFront();
			ctx.entities_loaded[0].sendMsg(msg);
		}
		else if (estadoMockControl == 2) {
			TEntityParseContext ctx;
			VEC3 posicion = VEC3(c_trans->getPosition().x, 1, c_trans->getPosition().z);
			c_trans->setPosition(posicion);
			ctx.root_transform = *c_trans;
			parseScene("data/prefabs/grenade.json", ctx); //cambiar prefab

			TMsgAssignGrenadeOwner msg;
			msg.h_owner = CHandle(this).getOwner();
			msg.source = c_trans->getPosition();
			msg.front = c_trans->getFront();
			ctx.entities_loaded[0].sendMsg(msg); 
		}
	}

	//GAMEPAD
	if (EngineInput.gamepad()._connected) {
		player_pos = player_pos + camera_front * EngineInput["front_"].value * speed * delta;
		dir += camera_front * EngineInput["front_"].value;
		player_pos = player_pos - camera_left * EngineInput["left_"].value * speed * delta;
		dir -= camera_left * EngineInput["left_"].value;
	}
	//MOVE PLAYER
	c_trans->setPosition(player_pos);

	if (dash <= dash_limit) {
		dash += delta;
		player_pos = player_pos + dir * 10.0f * delta;
		c_trans->setPosition(player_pos);
	}
	else if (EngineInput["dash_"].justPressed()) {
		dash = 0.0f;
		
	}
		
	if (dir != VEC3() && !aiming) { //ROTATE PLAYER WHERE HE WALKS
		//And also rotate the player on the direction its facing
		float yaw, pitch;
		c_trans->getAngles(&yaw, &pitch);
		dir.Normalize();
		float wanted_yaw = c_trans->getDeltaYawToAimTo(player_pos + dir);
		if(abs(wanted_yaw) > 0.01)
			c_trans->setRotation(QUAT::CreateFromYawPitchRoll(yaw + wanted_yaw * delta * rotation_speed, pitch, 0.0f));
	}
	else if(aiming) {  //ROTATE PLAYER WHERE THE CAMERA LOOKS
		float yaw, pitch;
		c_trans->getAngles(&yaw, &pitch);
		dir.Normalize();
		float wanted_yaw = c_trans->getDeltaYawToAimTo(player_pos + camera_front);
		if (abs(wanted_yaw) > 0.01)
			c_trans->setRotation(QUAT::CreateFromYawPitchRoll(yaw + wanted_yaw * delta * rotation_speed * 2.0f, pitch, 0.0f));
	}
	//Cambiar armas... cambiar
	if (EngineInput["fregona_"].justPressed()) { //pegar con la fregona 
		estadoMockControl = 0;
	}
	else if (EngineInput["banana_"].justPressed()) { //lanza platanos 
		estadoMockControl = 1;
	}
	else if (EngineInput["grenade_"].justPressed()) { //guindilla
		estadoMockControl = 2;
		
	}
	
}
*/

