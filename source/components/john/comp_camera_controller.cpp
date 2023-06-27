#include "mcv_platform.h"

#include "comp_camera_controller.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_name.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("camera_controller", TCompCameraController);

void TCompCameraController::debugInMenu() {
  ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 10.f);
  ImGui::DragFloat("Rotation speed", &rotation_speed, 0.0f, 0.f, 10.f);
} 

void TCompCameraController::load(const json& j, TEntityParseContext& ctx) {
	speed = j.value("speed", speed);
	rotation_speed = j.value("rotation_speed", rotation_speed);
}

void TCompCameraController::update(float delta) {
	c_trans = get<TCompTransform>();
	c_camera = get<TCompCamera>();
	
	MAT44 positionPlayer;

	if (isPressed('C')) {
		camera_type = !camera_type;
	}

	if (camera_type) {
		CEntity* h_player = (CEntity *)getEntityByName("Player");
		TCompTransform* c_trans = h_player->get<TCompTransform>();
		positionPlayer = c_trans->asMatrix();
		
		static float t = 1.0f; //delta
		t += 0.0001f * rotation_speed;
		float f = ((float)Render.width / (float)Render.height);
		c_camera->setAspectRatio(f);
		VEC3 camera_target, camera_position;
		VEC3::Transform(VEC3(0, 0, 15), positionPlayer, camera_target);
		VEC3::Transform(VEC3(0, 6, -10), positionPlayer, camera_position);
		c_camera->lookAt(camera_position, camera_target);
		
	}
	else {
		static float t = 1.0f; //delta
		t += 0.0001f * rotation_speed;
		float f = ((float)Render.width / (float)Render.height);
		c_camera->setAspectRatio(f);
		c_camera->lookAt(12.f * VEC3(cosf(t), 1.0f, sinf(t)), VEC3());
	}

	
}




