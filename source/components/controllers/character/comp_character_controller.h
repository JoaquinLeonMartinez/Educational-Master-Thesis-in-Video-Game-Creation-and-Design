#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/ai/graph/ai_controller.h"
#include "components/controllers/comp_sCart_controller.h"
#include "components/animation/comp_player_animation.h"
#include "modules/game/audio/audioEvent.h"
#include "components/controllers/comp_inventory.h"

class TCompCharacterController : public IAIController
{
public:
  VEC3 posCkeckpoint = VEC3(0, 0, 0); //cambiar posicion cuando Sepamos la posicion de inicio del jugador
  float maxLife = 100.f;
  float life = maxLife;
  bool endGame = false;
  PowerType power_selected;

  bool aiming = false;
  bool is_grounded = false;
  void Init();
	void update(float dt);
  void debugInMenu();
  void renderDebug();
  void load(const json& j, TEntityParseContext& ctx);
  static void registerMsgs();

  void setMeleeMultiplier(float newMulti);
  void setSpeed(float newSpeed);
  void setDashSpeed(float dashSpeed) { dash_speed = dashSpeed; }
  float getBaseSpeed();

  //Shopping Cart
  void dismount();
  //End Shopping Cart

  //Mesh Swap
  void SwapMesh(int state);
  //End Mesh Swap
  bool enabled = true;
  //POWER UPS
  void  applyPowerUp(float quantity, PowerUpType type, float extraBarSize);
  void heal();
  void healPartially(float health);
  //madness
  void restoreMadness();
  float getMaxMadness();
  bool getIsMounted() { return isMounted; }
  VEC3 getMovmentDirection() { return movementDirection; }

  /*bool unLockableBattery = false;
  bool unLockableTeleport = false;
  bool unLockableChilli = false;
  bool unLockableCoffe = false;*/
  bool cinematic = false;
  bool getAttacking() { return attacking; }
private:

  VEC3 movementDirection = VEC3().Zero;
	int typeRender = 2;
  //Shopping Cart
  bool isMounted = false;
  float interactRange = 3.5f;
  void interact();
  void mount(CHandle vehicle);
  //End Shopping Cart
  std::string statePrevious = "";
  float distance_to_aim = 20.0f;
  float speed = 5.0f;
  float inCombatTimer = 0.f;
  float inCombatDuration = 3.5f;
  
  bool last_frame_aiming = false;
  float rotation_from_aim = 0.0f;
  float base_speed = speed;
  float rotation_speed = 8.0f;
  float dash_speed = 35.0f;
  float dash_limit = 0.12f;
  float dash = dash_limit;
  float time_between_dashes = 1.0f;
  float time_to_next_dash = 0.0f;
  float jump_force = 7.f;
  float double_jump_force = 4.f;
  float falling_factor_hover = 0.25f;
  float moving_factor_hover = 1.0f;
  float damaged_force = 5.0f;
  bool can_double_jump = true;
  bool enemy_in_range = true;
  bool animation1Done = false;
  bool isBatteryAlive = false;
  bool isThrowingAnimationGoing = false;
  bool isCHARGED_MELEE_ATTACKGoing = false;
  bool isDRINKGoing = false;

	float invulnerabilityTimer = 0.f;
	float invulnerabilityTimeDuration = 1.0;

	float extintorMeshTimer = 0.f;
	float extintorMeshTimerDuration = 1.0;
	bool extintorActive = false;
	PowerType last_power_selected = PowerType::TELEPORT;

  //Audio Values
    AudioEvent damagedAudio;
    AudioEvent footSteps;
    AudioEvent footStepsSlow;
  //Audio Values

  //Melee Values
    bool meleeHit = false;
    bool alreadyAttacked = false;
  bool attackFirstExecution = true;
  bool attacking = false;
  float comboModifier = 2.0f;
  float meleeTimer = 0.f;
  float meleeDelay = 0.3f;
  float meleeTotalDuration = 0.4f;
  float meleeCurrentDuration = 0.f;
  float meleeDistance = 0.75f;
  float meleeDamage = 50.f;
  float meleeRadius = 1.25f;
  std::vector<CHandle> entitiesHit;
  float impactForceAttack = 8.0f;
  //End Melee Values

  //ChargedAttack Values
  float chargedAttack_chargeDelay = 1.f;
  float chargedAttack_damage = 75.f;
  float chargedAttack_radius = 5.f;
	float chargedAttack_impactForce = 20.0f;
  float chargedAttack_buttonPressThreshold = 0.2f;
  float chargedAttack_buttonPressTimer = 0.f;
  float chargedAttack_playerSpeed = 0.f;
  bool chargedAttack_releasing = false;
  bool chargedAttack_onAir = false;
  //End ChargedAttack Values

  //chilli values
  float hxChilli = 1.5f;
  float hyChilli = 1.f;
  float hzChilli = 2.f;
  float chilliDistance = 3.f;

  float rumble_time = 0.0f;
  
  bool cinematicFinish = false;

  float speedCinematicSpecial = 3.f;
  VEC3 targetTower = VEC3(-103, -0.057, -211);

  CHandle h_camera;

  //STATES
  void grounded(float delta);
  void dashing(float delta);
  void onAir(float delta);
  void damaged(float delta);
  void dead(float delta);
  void win(float delta);
  void noclip(float delta);
  void idleCinematic(float delta);
  void specialCinematic(float delta);

   //UTILS
  void treatRumble(float delta);
  void getInputForce(VEC3 &dir);
  void rotatePlayer(const VEC3 &dir, float delta, bool start_dash = false);
  bool isGrounded();
  void powerSelection();
  void shoot();
  void attack(float delta);
  void chargedAttack(float delta);

   //EVENTS
  void onCollision(const TMsgOnContact& msg);
  void onEnter(const TMsgEntityTriggerEnter& trigger_enter);
  void onDamageAll(const TMsgDamageToAll& msg);
 // void onDamage(const TMsgDamage& msg);
  void onGenericDamage(const TMsgDamage& msg);
  void onPowerUp(const TMsgPowerUp& msg);
  void onBatteryDeactivation(const TMsgBatteryDeactivates& msg);
  void onAnimationFinish(const TCompPlayerAnimator::TMsgPlayerAnimationFinished& msg);
  void onCinematic(const TMsgOnCinematic& msg);
  void onCinematicSpecial(const TMsgOnCinematicSpecial& msg);
  void onTrapWind(const TMsgTrapWind& msg);
  void onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg);
	void changeWeaponMesh(WeaponMesh weaponSelected);

  void onMeleeHit(const TMsgMeleeHit& msg);


  //?????
  void mounted(float delta);


  DECL_SIBLING_ACCESS();
};

