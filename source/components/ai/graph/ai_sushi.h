#ifndef _AI_SUSHI
#define _AI_SUSHI

#include "ai_controller.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/comp_transform.h"


class CAISushi : public IAIController
{

public:
  void Init();
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  static void registerMsgs();

  CHandle h_sender;
  CHandle h_bullet;

  Vector3 damageSource;

  //Battery Values
  float battery_time = 0.f;
  VEC3 battery_position = VEC3();
  float attractionForce = 20.f;
  float distanceToBattery;
  //End Battery Values

private:
	float init = true;
  //Pathing
  int nWaypoints = 0;
  std::vector<Vector3> positions;
  int wtpIndex = 0;
  Vector3 position;
  Vector3 currentPosition;
  Vector3 nextPoint;
  VEC3 impulse = VEC3();
  float randomTimer;
  float randomDelay = 1.f;

  float gravity = -9.81f;

  //external effects
  bool beingAttracted = false;

  //View Ranges
  float half_cone = 30.0f;
  float hearing_radius = 3.0f;
  float combatViewDistance = 30.0f;
  float viewDistance = 15.f;
  //End View Ranges

  //Core Values
  float life = 100.f;
  float patrolSpeed = 1.0f;
  float chaseSpeed = 4.0f;
  float rotationSpeed = 4.0f;
  float damageStunDuration = 0.1f;
  float damageStunTimer = damageStunDuration;
  float damageStunBackwardsSpeed = 1.5f;
  float distanceCheckThreshold = 0.5f;
  //End Core Values

  //Melee Values
  int meleeComboProbability = 33;
  float melee1Damage = 15.f;
  float melee2Damage = 15.f;
  float melee3Damage = 20.f;
  float meleeDistance = 3.f;
  float meleeDelay = 1.7f;
  float meleeTimer = meleeDelay;
  float meleeComboDelayReducedFactor = 0.5f;
  //End Melee Values

  //Charge Values
  int chargeProbability = 40;
  float chargeSpeed = 5.0f;
  float chargeDamage = 20.0f;
  //Delay will be dictated by the animation length
  float chargeDelay = 2.f;
  float chargeDuration = 1.5f;
  float chargeElapsed = 0.f;
  float chargeTimer = chargeDelay;
  VEC3 chargeObjective = VEC3();
  VEC3 chargePoint = VEC3();
  //End Charge Values

  //Jump Charge Values
  VEC3 jumpPosition = VEC3().Zero;
  int jumpChargeProbability = 30;
  float jumpSpeed = 3.0f;
  float jumpHeight = 5.0f;
  float jumpChargeSpeed = 5.0f;
  float jumpChargeDamage = 20.0f;
  //End Jump Charge Values

  //Salute Values
  int saluteProbability = 60;
  float saluteDuration = 2.f;
  float saluteElapsed = 0.f;
  //End Salute Values

  //Blocking Values
  bool hasBlocked = false;
  bool isBlocking = false;
  int blockProbability = 15;
  float blockDuration = 3.f;
  float blockRemaining = blockDuration;
  //End Blocking Values

  //Orbit Values
  int orbitProbability = 33;
  float orbitSpeed = 1.0f;
  //End Orbit Values

  //Utils
  CHandle h_player;

  //Flags
  bool inCombat = false;
  bool blocking = false;
  std::string last_state = "";

  //Functions
  bool isView();
  template <typename T>
  void Send_DamageMessage(CEntity* entity, float dmg);
  void LookAt_Player(TCompTransform* c_trans, TCompTransform* player_position, float dt);
  void onCollision(const TMsgOnContact& msg);
  void onDamageInfoMsg(const TMsgDamageToEnemy& msg);
  void onGravity(const TMsgGravity& msg);
  bool isGrounded();

  //Pathing States
  void OnAir(float dt);
  void SeekwptState(float dt);
  void NextwptState(float dt);
  //Combat States
  void ChaseState(float dt);
  void IdleWarState(float dt);
  void BlockingState(float dt);
  void PrepareCharge(float dt);
  void PrepareJumpCharge(float dt);
  void OrbitLeftState(float dt);
  void OrbitRightState(float dt);
  void JumpChargeState(float dt);
  void ChargeState(float dt);
  void SaluteState(float dt);
  void Melee1State(float dt);
  void Melee2State(float dt);
  void Melee3State(float dt);
  //Involuntary States
  void ImpactState(float dt);
  void DeadState(float dt);
  void BatteryEffectState(float dt);


  DECL_SIBLING_ACCESS();
};

#endif _AI_SUSHI