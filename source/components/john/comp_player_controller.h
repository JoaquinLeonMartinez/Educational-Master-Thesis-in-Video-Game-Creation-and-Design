#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/ai/ai_controller.h"

class TCompPlayerController : public IAIController {

  float speed = 2.0f;
  float rotation_speed = 4.0f;
  float dash_speed = 15.0f;
  float dash_limit = 0.4f;
  float dash = dash_limit;
  float jump_force = 10.0f;
  float double_jump_force = 10.0f;
  float falling_factor_hover = 0.25f;
  float moving_factor_hover = 0.5f;
  bool can_double_jump = true;

  VEC3 impulse = VEC3();

  int weapon_selected = 1;
  float rumble_time = 0.0f;

  CHandle h_camera;
  
  void grounded(float delta);
  void dashing(float delta);
  void onAir(float delta);
  void hovering(float delta);
  void treatRumble(float delta);
  void shoot();
  void getInputForce(VEC3 &dir);
  void rotatePlayer(const VEC3 &dir, float delta, bool immediate = false);
  bool isGrounded();
  void weaponSelection();
  void attack();

  DECL_SIBLING_ACCESS();

public:
	bool aiming = false;

	void Init();
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
};