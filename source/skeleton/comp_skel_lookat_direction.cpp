#include "mcv_platform.h"
#include "comp_skel_lookat_direction.h"
#include "components/common/comp_transform.h"
#include "skeleton/comp_skeleton.h"
#include "cal3d/cal3d.h"
#include "cal3d2engine.h"
#include "entity/entity_parser.h"
#include "game_core_skeleton.h"
#include "components/controllers/character/comp_character_controller.h"
#include "input/module_input.h"

DECL_OBJ_MANAGER("skel_lookat_direction", TCompSkelLookAtDirection);

void TCompSkelLookAtDirection::load(const json& j, TEntityParseContext& ctx) {
  /*if( j.count("target"))
    target = loadVEC3( j["target"] );*/
  amount = j.value("amount", amount);
  angle = j.value("angle", angle);
  target_transition_factor = j.value("target_transition_factor", target_transition_factor);
}


void TCompSkelLookAtDirection::getInputDir(VEC2 & dir) {
    if (EngineInput["front_"].isPressed()) {
      dir += VEC2(0,1);
    }
    if (EngineInput["back_"].isPressed()) {
      dir -= VEC2(0, 1);
    }
    if (EngineInput["left_"].isPressed()) {
      dir -= VEC2(1, 0);
    }
    if (EngineInput["right_"].isPressed()) {
      dir += VEC2(1, 0);
    }

    //GAMEPAD
    if (EngineInput.gamepad()._connected) {
      //TO CORRECT A BUG WITH GAMEPAD CONNECTED WITH KEYBOARD
      if (!EngineInput["w"].isPressed())
        dir += VEC2(0, 1) * EngineInput["front_"].value;

      if (!EngineInput["a"].isPressed())
        dir += VEC2(1, 0) * EngineInput["left_"].value;
    }

    dir.Normalize();
  

}

void TCompSkelLookAtDirection::update(float dt) {
  TCompSkeleton* c_skel = h_skeleton;

  VEC2 input_dir = VEC2::Zero;
  getInputDir(input_dir);

  TCompCharacterController* c_c = get < TCompCharacterController>();
  if (!c_c) {
    return;
  }
  float targ_amount = 0.0f;
  std::string state = c_c->getState();
  if (input_dir != VEC2::Zero && c_c->aiming && strcmp(state.c_str(),"ON_AIR") != 0) {
    float input_angle = atan2(input_dir.y, input_dir.x) - (M_PI / 2.0f);
    targ_amount = input_angle / (-M_PI / 2.0f);

    if (targ_amount > 1.0f) {//estamos caminando hacia atras
      input_angle += M_PI;
      targ_amount = input_angle / (-M_PI / 2.0f);
    }
  }
  float prev_amount = amount;
  amount = prev_amount + sign(targ_amount - amount) * dt * 4.0f;
  if (prev_amount > targ_amount && amount < targ_amount) {
    amount = targ_amount;
  }
  else if (prev_amount < targ_amount && amount > targ_amount) {
    amount = targ_amount;
  }
  if (abs(amount) < 0.05 && targ_amount == 0.0f) {
    amount = 0.0f;
  }


  if (c_skel == nullptr) {
    // Search the parent entity by name
    CEntity* e_entity = CHandle(this).getOwner();
    if (!e_entity)
      return;
    // The entity I'm tracking should have an skeleton component.
    h_skeleton = e_entity->get<TCompSkeleton>();
    assert(h_skeleton.isValid());
    c_skel = h_skeleton;
  }

  // The cal3d skeleton instance
  CalSkeleton* skel = c_skel->model->getSkeleton();

  // The set of bones to correct
  VEC3 up(0, 1, 0);
  auto core = (CGameCoreSkeleton*)c_skel->model->getCoreModel();
  float angle_rads = deg2rad(angle);
  for (auto& it : core->lookat_direction_corrections)
    it.applyLocal(skel, up, angle_rads * amount) ;
}

void TCompSkelLookAtDirection::renderDebug() {
  drawMesh(Resources.get("unit_wired_cube.mesh")->as<CMesh>(), MAT44::CreateTranslation(target), VEC4(1, 0, 0, 1));
}

void TCompSkelLookAtDirection::debugInMenu() {
  ImGui::InputFloat3("Target", &target.x);
  ImGui::LabelText( "Target Name", "%s", target_entity_name.c_str() );
  ImGui::DragFloat("Amount", &amount, 0.01f, 0.f, 1.0f);
  ImGui::DragFloat("Angle", &angle, 0.01f, -90.f, 90.f);
  ImGui::DragFloat("Transition Factor", &target_transition_factor, 0.01f, 0.f, 1.0f);
}
