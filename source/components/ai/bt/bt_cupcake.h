#ifndef _BT_CUPCAKE
#define _BT_CUPCAKE

#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "bt_controller.h"
#include "geometry/curve.h"
#include "modules/game/audio/audioEvent.h"

class CBTCupcake : public BTController {
	CHandle h_player;
	DECL_SIBLING_ACCESS();
public:
	void create(string s); //init
	void updateBT();
	 //acciones == hojas

	void setCurve(const CCurve* curve);

	void setLengthCone(float length_cone);
	
	void setHalfCone(float half_cone);

	void setLife(float life_);
	float getLife();
	int actionChangeWpt();
	int actionSeekWpt();
	int actionAttack();
	int actionGeneratePoint();
	int actionFollowPoint();
	int actionChangeState();

	int actionDivide();
	int actionDeath();
  int actionDeathStay();
	int actionGravityReceived();
	int actionImpactReceived();
	int actionRecoilReceived();


	//condiciones para establecer prioridades

	bool conditionView();
	bool conditionWptClose();
	bool conditionOrbitPointClose();
	bool conditionRandom();


	bool conditionDeath();
  bool conditionDeathAnimation();
	bool conditionDivide();
	bool conditionGravityReceived();
	bool conditionImpactReceived();
	bool conditionRecoilReceived();
	bool conditionTimer();


	void generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc);
	void onTMsgToCupcakeSon(const TMsgToCupcakeSon& msg);
	void onCollision(const TMsgOnContact& msg);
	void onGravity(const TMsgGravity& msg);
	void onDamageToAll(const TMsgDamageToAll& msg);
	void onFireAreaEnter(const TMsgFireAreaEnter& msg);
	void onFireAreaExit(const TMsgFireAreaExit& msg);
	void onGenericDamageInfoMsg(const TMsgDamage& msg);
	void onCreated(const TMsgEntityCreated& msg);
	void onCheckin(const TMsgSpawnerCheckin& msg);
	void onBlackboardMsg(const TMsgBlackboard& msg);
	void onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg);
	void onDeleteTrigger(const TMsgDeleteTrigger& msg);
	static void registerMsgs();

	template <typename T>
	void Send_DamageMessage(CEntity* entity, float dmg);

	void load(const json& j, TEntityParseContext& ctx);

	void movement(VEC3 target, bool seek);

	bool isGrounded();
	bool isView(float distance);

	void generatePoints();
	void renderDebug();

	int damage = 40.f;//6.25f;
	int fireDamage = 15;
	int currentDamage = damage;
  bool death_animation_started = false;
	bool invulnerable = false;
	float recienNacidoTimer = 2.0f; //tiempo que sera invulnerable el cupcake que acaba de nacer

	//CURVE  VALUES
	float	mTravelTime = 0.22f;
	float ratio = 0.f;
	const CCurve* _curve = nullptr;
	std::vector<VEC3> _knots;
	//END CURVE VALUES
	std::string getNameCurve();
private:
    AudioEvent voice;



	CHandle _mySpawner;
	bool _initiallyPaused = false;


	int nWaypoints = 0;
	std::vector<Vector3> positions;
	int wtpIndex = 0;

	Vector3 position;
	Vector3 currentPosition;
	Vector3 nextPoint;
	VEC3 initialPos = VEC3(0, 0, 0); //se actualiza al crearse

	float impactForceAttack = 5.0f;

	//timer send damage
	float damageTimer = 0;
	float damageDelay = 1.0;

	//navmesh values
	bool use_navmesh = true;
	float reevaluatePathTimer= 0;
	float reevaluatePathDelay = 1.0;
	int navMeshIndex = 0;
	Vector3 nextNavMeshPoint = VEC3().Zero;
	std::vector<VEC3> navmeshPath;
	unsigned int navmeshPathPoint;
	bool recalculateNavmesh = false;
	std::vector<VEC3> patrolPoints;

	//end navmesh values

	bool orbitting = false;
	bool initialExecution = true;
	bool noOrbitPoint = true;
	bool attacking = false;
	bool impacted = false;
	bool recoiled = false;
	bool insightPlayer = false;
	//gameplay parameters
	int num_of_divisions = 1; //times will you divide your self
	float life = 100.f;
	float sonLife = 50.0f;
	int num_of_sons = 2; //number of sons
	int spawnRange = 5; //spawn range of sons
	float orbitRadious = 3.0f;
	int minDistanceOrbit = 4.0; //for random
	int number_of_orbit_points = 4; //for random
	bool jumpingCupcake = false; //type of cupcake jumping or hide
	bool jump = true; //true
	float limitTimeStun = 0.1f;
	float timerStun = limitTimeStun;
	float changeStateProb = 50;

	//fire values
	float normalScale = 1.f;
	float fireScale = 2.f;

	//End Fear Values

	float evaluateStateTime = 2.f; //cada cuantos segundos reevalua si esconderse o no
	float evaluateState = evaluateStateTime;
	float evaluateAttack = 5.0f; //cada cuantos segundos reevalua si atacar o no
	float evaluateAttackValue = evaluateAttack;

	//son collider
	float sonRadius = 0.22f;
	float sonHeight = 0.25f;

	int orbited_waypoints = 0;
	int max_orbited_points = 3; //cada cuantos orbitpoints atacara si o si

	//Battery Values
	bool beingAttracted = false;
	float battery_time = 0.f;
	VEC3 battery_position = VEC3().Zero;
	VEC3 resultingForce = VEC3().Zero;
	float attractionForce = 60.f;
	float distanceToBattery = 0.f;
	float repulsionDuration = 0.4f;
	float repulsionTimer = 0.f;

	//orbit
	std::vector<VEC3> orbitPositions;
	int orbitIndex = 0;
	VEC3 nextOrbitPoint = VEC3().Zero;

	//blackboard
	bool slotsAvailable = false;
	bool player_dead = false;

	bool inBlackboard = false;

	float resetSlotDuration = 3.0f;
	float resetSlotTimer = 0.0f;


	//probabilities
	float orbitProbability = 80;
	float attackProbability = 50;
	float stopOrbitProbability = 0;

	//external effects
	VEC3 bullet_position = VEC3();
	VEC3 bullet_front = VEC3();
	float bullet_impulse = 60.0;
	//battery
	/*
	bool beingAttracted = false;
	VEC3 battery_position = VEC3();
	float battery_time = 0.f;
	float distanceToBattery;
	float attractionForce = 60.f;*/

	//jump
	bool firstImpact = true;
	float jumpForce = 4.0f; //5
	float recoilJumpForce = 5.0f; //5

	//movement
	float distanceCheckThreshold = 1.0f;
	float forgetAttackingDistance = 20.f;
	//speeds
	float stunSpeed = 1.5f; //1.5
	float speed = 5.0f; //5
	float hideSpeed = 10.f; //10
	float twistSpeed = 350.f; //5 //radianes
	float batterySpeed = 20.f;

	float impulseStrenghtSecondSon = 7.f;

	float cone_vision = 20.f;
	float enemyRadiousView = 12.0f;
	bool view_point = false;
	float length_cone = 20.0f;
	float half_cone = 45.0f;
	float height_range = 3.0f;
	bool isDeadForFallout = false;
	bool isDeadForTrigger = false;
	bool checkHeight();

	CHandle h_sender;
	CHandle h_bullet;

	std::string pathCurve = "";

  std::string state = "";
  std::string getState();

};



#endif _BT_CUPCAKE