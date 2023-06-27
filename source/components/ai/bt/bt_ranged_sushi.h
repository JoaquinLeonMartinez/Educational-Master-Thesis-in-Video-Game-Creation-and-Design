#ifndef _BT_RANGED_SUSHI
#define _BT_RANGED_SUSHI

#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "bt_controller.h"
#include "geometry/curve.h"
#include "modules/game/audio/audioEvent.h"

class CBTRangedSushi : public BTController {


  DECL_SIBLING_ACCESS();
public:
	void updateBT(); //init
	void create(string); //init
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
  //void debugInMenu();
  void renderDebug();
  static void registerMsgs();
  std::string getState();

  //Actions
  int actionNextWaypoint();
  int actionSeekWaypoint();
  int actionSalute();
  int actionSingleShot();
  int actionSpreadShot();
  int actionBurstShot();
  int actionLeap();
  int actionBounce();
  int actionChase();
  int actionRetreat();
  //int actionOrbitRight();
  //int actionOrbitLeft();
  int actionOnAir();
  int actionImpactReceived();
  int actionGravityReceived();
  int actionFear();
  int actionDeath();
  int actionDeathStay();
  int actionIdle();
  int actionIdleCombat();
  int actionDecoy();
  //End Actions

  //Conditions
  bool conditionPlayerInView();
  bool conditionChase();
  bool conditionRetreat();
  bool conditionSalute();
  bool conditionLeap();
  bool conditionBounce();
  bool conditionShoot();
  //bool conditionOrbit();
  bool conditionOnAir();
  bool conditionImpactReceived();
  bool conditionGravityReceived();
  bool conditionFear();
  bool conditionDeath();
  bool conditionDeathAnimation();
  bool conditionDecoy();

  void setHeightRange(float height);
	void setCurve(const CCurve* curve);
	void onBlackboardMsg(const TMsgBlackboard& msg);
	std::string getNameCurve();
	void setViewDistance(float value);
	void setHalfCone(float halfCone);
  //End Conditions
private:
    AudioEvent _footSteps;
    float initialExecution = true;
  //Pathing
  int nWaypoints = 0;
  std::vector<Vector3> positions;
  int wtpIndex = 0;
  Vector3 position;
  Vector3 currentPosition;
  Vector3 nextPoint;
  VEC3 impulse = VEC3();
  int directionJump;//0 right, 1 left
  bool death_animation_started = false;
  enum class States {
    undefined = 0,
	Idle,
	IdleCombat,
	NextWaypoint,
    SeekWaypoint,
	Salute,
	Shoot,
	Leap,
	Bounce,
	Chase,
	Retreat,
    OrbitRight,
    OrbitLeft,
    OnAir,
    ImpactReceived,
    GravityReceived,
	FearReceived
  };
  States previousState = States::undefined;
  States currentState = States::undefined;

  enum class ShotType {
	  Single = 0,
	  Spread,
	  Burst,
	  Random
  };

  AudioEvent _audioPlaying;// = EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Death3D");
  bool hasPlayedTripleThrowAudio = false;
  //navmesh values
  bool use_navmesh = true;
  float reevaluatePathTimer = 0;
  float reevaluatePathDelay = 0.2f;
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
  float shootRange = 20.f;
  float retreatDistance = 10.f;
  //End View Ranges

  //Core Values
  float life = 200.f;
  float patrolSpeed = 1.0f;
  float chaseSpeed = 4.0f;
  float retreatSpeed = 2.0f;
  float rotationSpeed = 90.0f;
  float damageStunDuration = 0.5f;
  float damageStunTimer = 0.f;
  bool damaged = false;
  float damageStunBackwardsSpeed = 1.5f;
  float distanceCheckThreshold = 0.5f;
  float _actionDelay = 2.f;
  float _actionTimer = _actionDelay;
  bool _initiallyPaused = false;
  //End Core Values

  //Impact Values
  VEC3 direction_to_damage;
  float impactForce = 5.f;
  //End Impact Values
  
  //Salute Values
  bool hasSaluted = false;
  int saluteProbability = 60;
  float saluteDuration = 2.f;
  float saluteElapsed = 0.f;
  //End Salute Values

  //Shoot Values
  float _bulletDamage = 5.f;
  float _bulletForce = 2.5f;
  float _playerOffset = 7.5f;
  int _shotsFired = 0;
  float _burstDelay = .3f;
  float _burstTimer = 0.f;
  //End Shoot Values

  //Leap Values
  int _leapProbability = 40;
  bool _isLeaping = false;
  bool _hasFired = false;
  float _leapStrength = 10.f;
  float _shootDelay = 1.5f;
  float _shootTimer = _shootDelay;
  VEC3 _jumpSource = VEC3().Zero;
  //End Leap Values

  //Bounce Values
  bool _hasBounced = false;
  float _bounceStrength = 17.f;
  VEC3 _bounceSource = VEC3().Zero;
  //End Bounce Values

  //Orbit Values
  /*int _orbitProbability = 33;
  float _orbitSpeed = 4.5f;
  float _frontalSpeed = 2.0f;
  float _orbitDistance = 15.f;
  float _orbitCooldown = 4.f;
  float _orbitCooldownTimer = 0.f;*/
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

  //Fear Values
  bool _onFireArea = false;
  float _fearSpeed = 5.5f;
  float _fearDuration = 2.f;
  float _fearTimer = 0.f;
  VEC3 _fearOrigin = VEC3().Zero;
  //End Fear Values

  //Decoy Values
  bool _decoyTriggered = false;
  float _decoyCooldown = 10.f;
  float _decoyCooldownTimer = 0.f;
  int _decoyProbability = 30;
  float _decoyTeleportDistance = 5.f;
  VEC3 _decoyOrigin = VEC3().Zero;
  //End Decoy Values
  float height_range = 3.0f;
  bool hayCamino = true;
  bool isDeadForTrigger = false;
  bool isDeadForFallout = false;
  //Utils
  CHandle h_player;
  CHandle h_sender;
  CHandle h_bullet;
  VEC3 damageSource;
  //End Utils

  //Functions
  void OnAir();
  bool isView();
  template <typename T>
  void Send_DamageMessage(CEntity* entity, float dmg);
  bool isGrounded();
  bool isHole(VEC3 jump);
  bool isOtherEnemyInSide();
  void onCollision(const TMsgOnContact& msg);
  void onGenericDamageInfoMsg(const TMsgDamage& msg);
  void onGravity(const TMsgGravity& msg);
  void onFireAreaEnter(const TMsgFireAreaEnter& msg);
  void onFireAreaExit(const TMsgFireAreaExit& msg);
  void onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg);
  void onDeleteTrigger(const TMsgDeleteTrigger& msg);
  VEC3 getLeapDirection();
  void shoot(ShotType type);
  void singleShot();
  void spreadShot();
  bool rollDice(int probability);
  void generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc);
  bool isPlayerInNavmesh();
  bool obstacleInJump();
  bool checkHeight();
  VEC3 calculatePositionGround();
  //Flags
  bool inCombat = false;

  //Blackboard
  bool player_dead = false;
  bool slotsAvailable = false;
  bool checkBlackboard();
	float resetSlotDuration = 5.0f;
	float resetSlotTimer = 0.0f;

	std::string pathCurve = "";
  //flag patrol curve or point
  

	

	//curve values
	const CCurve* _curve = nullptr;
	std::vector<VEC3> _knots;
	float	mTravelTime = 0.22f;
	float ratio = 0.f;
	//end curve values
	
  /* DEPRECATED */
  //void LookAt_Player(TCompTransform* c_trans, TCompTransform* player_position);
};

#endif _BT_RANGED_SUSHI