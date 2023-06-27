#include "mcv_platform.h"
#include "comp_parabolic_launch.h"
#include "components/common/comp_transform.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/controllers/camera/comp_aim_controller.h"
#include "components/common/comp_render.h"
#include "input/input.h"
#include "components/powers/comp_madness.h"
#include "ui/widgets/ui_image.h"
#include "ui/module_ui.h"

DECL_OBJ_MANAGER("parabolic_launch", TCompParabolicLaunch);

void TCompParabolicLaunch::debugInMenu() {
  ImGui::DragFloat("Velocity", &v0, 0.1f, 1.f, 100.f);
  /*ImGui::DragFloat("Sensitivity", &_sensitivity, 0.001f, 0.001f, 0.1f);
  ImGui::DragFloat("Inertia", &_ispeed_reduction_factor, 0.001f, 0.7f, 1.f);
  ImGui::LabelText("Curr Speed", "%f", _ispeed);*/
}

void TCompParabolicLaunch::load(const json& j, TEntityParseContext& ctx){
	v0 = j.value("velocity", v0);
}

void TCompParabolicLaunch::renderDebug()
{
 
}

void TCompParabolicLaunch::update(float dt) {
  if (!h_player.isValid()) {
    h_player = getEntityByName("Player");
    return;
  }
  CEntity* e_player = h_player;
  TCompTransform* c_trans_pl = e_player->get<TCompTransform>();

  TCompCollider* comp_collider = e_player->get<TCompCollider>();
  if (!comp_collider || !comp_collider->controller)
    return;

  float playerRadius = (float)comp_collider->controller->getRadius();

  TCompTransform* c_trans = get<TCompTransform>();
  c_trans->setPosition(c_trans_pl->getPosition() + (c_trans_pl->getFront() * (playerRadius)));

  //get yaw from player
  float p_yaw, p_pitch;
  c_trans_pl->getAngles(&p_yaw, &p_pitch);

  float yaw, pitch;
  c_trans->getAngles(&yaw, &pitch);

  pitch = p_pitch;
  c_trans->setAngles(p_yaw, pitch);

	TCompCharacterController* c_player = ((CEntity*)h_player)->get<TCompCharacterController>();
	CEntity* e_inventory = getEntityByName("Inventory");
    TCompInventory* inventory = e_inventory->get<TCompInventory>();
	if (c_player->aiming && c_player->power_selected == PowerType::BATTERY && inventory->getBattery()) {
		CEntity* e_camera = getEntityByName("PlayerCamera");
		TCompTransform* transformPlayer = ((CEntity*)e_player)->get<TCompTransform>();
		TCompTransform* transformC_aim = e_camera->get<TCompTransform>();
		// 1.80  mide pj
		float yaw, pitch;
		transformC_aim->getAngles(&yaw, &pitch);
		float yawPj, pitchPj;
		transformPlayer->getAngles(&yawPj, &pitchPj);
		loadValuesParabola(heightPj, pitch, yawPj);
		/*UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
		mirilla->getParams()->visible = false;*/
  }

	
}

void TCompParabolicLaunch::calculateParabolicPoints() {
	curve_dynamic = new CCurve();
  iterations = 100;
  bool found = false;
	for (int i = 1; i < iterations; i++) {
		float y = h + v0y * (i / v0x) - 0.5 * (9.81) * pow((i / v0x), 2);
		const VEC3 operacion = VEC3(0, y, i);

    if (y <= h && !found) {
      iterations = i + 50;
      found = true;
    }

		if (!h_player.isValid()) { return; }

		TCompTransform* cTargetTransform = ((CEntity*)h_player)->get<TCompTransform>();
		const MAT44 mRotation = MAT44::CreateFromYawPitchRoll(yaw, 0.f, 0.f);
		const MAT44 mTranslation = MAT44::CreateTranslation(cTargetTransform->getPosition() + cTargetTransform->getFront() * 0.5f);
		MAT44 resultMatrix = (mRotation * mTranslation * MAT44::Identity);
		VEC3 result;
		Vector3::Transform(operacion, resultMatrix, result);
		curve_dynamic->addKnot(result);
	}
}

void TCompParabolicLaunch::loadValuesParabola(float new_h, float new_pitch,float new_yaw) {
	h = new_h;
	pitch = new_pitch;
	yaw = new_yaw;
	v0x = v0 * cosf(-1 * pitch);
	v0y = v0 * sinf(-1 * pitch);

	calculateParabolicPoints();

  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesAimCurve");
    constants.x = h;
    constants.y = v0;
    constants.z = pitch;
    constants.w = iterations;
    buf->updateGPU(&constants);
  }
}
