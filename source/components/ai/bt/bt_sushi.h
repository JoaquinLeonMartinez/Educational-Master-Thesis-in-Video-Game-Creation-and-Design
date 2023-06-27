#ifndef _BT_SUSHI
#define _BT_SUSHI

#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "bt_controller.h"
#include "geometry/curve.h"
#include "modules/game/audio/audioEvent.h"

class CBTSushi : public BTController {


  DECL_SIBLING_ACCESS();
public:
  void create(string); //init
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void renderDebug();
  static void registerMsgs();
  void updateBT(); //init
  std::string getState();

  //Actions
  int actionNextWaypoint();
  int actionSeekWaypoint();
  int actionSalute();
  int actionPrepareJumpCharge();
  int actionJumpCharge();
  int actionPrepareCharge();
  int actionCharge();
  int actionChase();
  int actionBlock();
  int actionBlockOrbitRight();
  int actionBlockOrbitLeft();
  int actionMelee1();
  int actionMelee2();
  int actionMelee3();
  int actionOnAir();
  int actionImpactReceived();
  int actionGravityReceived();
  int actionFear();
  int actionDeath();
  int actionDeathStay();
  int actionIdleCombat();
  //End Actions

  //Conditions
  bool conditionPlayerInView();
  bool conditionSalute();
  bool conditionMelee();
  bool conditionJumpCharge();
  bool conditionCharge();
  bool conditionBlock();
  bool conditionBlockOrbit();
  bool conditionCombo3();
  bool conditionCombo2();
  bool conditionOnAir();
  bool conditionImpactReceived();
  bool conditionGravityReceived();
  bool conditionFear();
  bool conditionDeath();
  bool conditionDeathAnimation();
  bool conditionChase();
  std::string getNameCurve();
	void setCurve(const CCurve* curve);
	void setHeightRange(float height);
	void setViewDistance(float distance);
	void setHalfCone(float halfCone);
	void setSlotsAvailable(bool slotsAvailable_) { slotsAvailable = slotsAvailable_; }
  //End Conditions
private:
  float initialExecution = true;
  //Pathing
  int nWaypoints = 0;
  std::vector<Vector3> positions;
  int wtpIndex = 0;
  Vector3 currentPosition;
  Vector3 nextPoint;
  VEC3 impulse = VEC3();
  AudioEvent _jumpChargeAudio;// = EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Death3D");
  AudioEvent _chargeAudio;// = EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Death3D");
  AudioEvent _footSteps;
  AudioEvent _audioPlaying;
  bool _chargeAudioPlaying = false;
  bool _jumpChargeAudioPlaying = false;
  bool death_animation_started = false;

	//curve values
  const CCurve* _curve = nullptr;
  std::vector<VEC3> _knots;
	float	mTravelTime = 0.22f;
	float ratio = 0.f;
	//end curve values
  enum class States {
    undefined = 0,
    NextWaypoint,
    SeekWaypoint,
	IdleCombat,
	Salute,
    PrepareJumpCharge,
    JumpCharge,
    PrepareCharge,
    Charge,
    Chase,
    Block,
    BlockOrbitRight,
    BlockOrbitLeft,
    Melee1,
    Melee2,
    Melee3,
    OnAir,
    ImpactReceived,
    GravityReceived,
	FearReceived
  };
  States previousState = States::undefined;
  States currentState = States::undefined;

  //navmesh values
  bool use_navmesh = false;
  float reevaluatePathTimer = 0;
  float reevaluatePathDelay = 0.1;
  int navMeshIndex = 0;
  Vector3 nextNavMeshPoint = VEC3().Zero;
  std::vector<VEC3> navmeshPath;
  unsigned int navmeshPathPoint;
  bool recalculateNavmesh = false;
  std::vector<VEC3> patrolPoints;
  //end navmesh values

  //View Ranges
  float half_cone = 30.0f;
  float hearing_radius = 3.0f;
  float combatViewDistance = 30.0f;
  float viewDistance = 15.f;
  float height_range = 3.0f;
  //End View Ranges

  //Core Values
  float life = 300.f;
  float patrolSpeed = 1.0f;
  float chaseSpeed = 3.0f;
  float rotationSpeed = 90.0f;
  float damageStunDuration = 0.5f;
  float damageStunTimer = 0.f;
  bool damaged = false;
  float damageStunBackwardsSpeed = 1.5f;
  float distanceCheckThreshold = 1.0f;
  float randomTimer = 0.f;
  float randomDelay = 2.f;
  bool _initiallyPaused = false;
  //End Core Values

  //Melee Values
  int meleeComboProbability = 33;
  float melee1Damage = 15.f;
  float melee2Damage = 15.f;
  float melee3Damage = 20.f;
  bool melee1Executed = false;
  bool melee2Executed = false;
  float meleeDistance = 3.f;
  float meleeDelay = 1.7f;
  float meleeTimer = meleeDelay;
  //End Melee Values

  //Charge Values
  bool collided = false;
  int chargeProbability = 40;
  float chargeSpeed = 5.0f;
  float chargeDamage = 20.0f;
  //Delay will be dictated by the animation length
  float chargeDelay = 1.f;
  float chargeDuration = 1.5f;
  float chargeElapsed = 0.f;
  float chargeTimer = chargeDelay;
  float chargeCooldown = 3.f;
  float chargeCooldownTimer = 0.f;
  VEC3 chargeObjective = VEC3();
  VEC3 chargePoint = VEC3();
  float chargeDistanceThreshold = 2.f;
  //End Charge Values

  //Jump Charge Values
  VEC3 jumpPosition = VEC3().Zero;
  int jumpChargeProbability = 30;//30
  float jumpSpeed = 3.0f;
  float jumpHeight = 5.0f;
  float jumpChargeSpeed = 5.0f;
  float jumpChargeDamage = 20.0f;
  float jumpChargeCooldown = 6.f;
  float jumpChargeCooldownTimer = 0.f;
  float jumpChargeDistanceThreshold = 3.f;
  float explosionRadius = 3.f;
  float explosionDamage = 20.f;
  //End Jump Charge Values

  //Salute Values
  bool hasSaluted = false;
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

	float resteBlockPorbabiliy;
	float resteBlockPorbabiliyTimer = 1.0;

  //Orbit Values
  int orbitProbability = 33;
  float orbitSpeed = 1.0f;
  //End Orbit Values

  //Battery Values
  bool beingAttracted = false;
  float battery_time = 0.f;
  VEC3 battery_position = VEC3().Zero;
  VEC3 resultingForce = VEC3().Zero;
  float attractionForce = 60.f;
  float distanceToBattery = 0.f;
  float repulsionDuration = 0.4f;
  float repulsionTimer = 0.f;
  //End Battery Values

  //Impact Values
  VEC3 direction_to_damage;
  float impactForce = 5.f;
  //End Impact Values

  //Fear Values
  bool _onFireArea = false;
  float _fearSpeed = 5.5f;
  float _fearDuration = 2.f;
  float _fearTimer = 0.f;
  VEC3 _fearOrigin = VEC3().Zero;
  //End Fear Values

  //Utils
  CHandle h_player;
  CHandle h_sender;
  CHandle h_bullet;
  Vector3 damageSource;
  //End Utils

  //Functions
  void OnAir();
  bool isView();
  template <typename T>
  void Send_DamageMessage(CEntity* entity, float dmg);
  bool isGrounded();
  void onCollision(const TMsgOnContact& msg);
  void onGenericDamageInfoMsg(const TMsgDamage& msg);
  void onGravity(const TMsgGravity& msg);
  void onFireAreaEnter(const TMsgFireAreaEnter& msg);
  void onFireAreaExit(const TMsgFireAreaExit& msg);
  void onBlackboardMsg(const TMsgBlackboard& msg);
  void onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg);
  void onDeleteTrigger(const TMsgDeleteTrigger& msg);
  //void onDamageAll(const TMsgDamageToAll& msg);
  bool rollDiceCharge();
  bool rollDiceJumpCharge();
  bool rollDice(int probability);
  void generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc);
  bool checkHeight();
 VEC3 calculatePositionGround();
  //Flags
  bool inCombat = false;
  bool blocking = false;
  bool nopath = false;
  //Blackboard
  bool player_dead = false;
	bool slotsAvailable = false;
  bool checkBlackboard();
	float resetSlotDuration = 5.0f;
	float resetSlotTimer = 0.0f;

  //attack
  float impactForceAttack = 8.0f;
  float impactReceived;
  bool isPlayerInNavmesh();
  bool hayCamino = true;
  bool isDeadForFallout = false;
  bool isDeadForTrigger = false;
  std::string pathCurve = "";


  /* DEPRECATED */
  //void LookAt_Player(TCompTransform* c_trans, TCompTransform* player_position);
};

#endif _BT_SUSHI