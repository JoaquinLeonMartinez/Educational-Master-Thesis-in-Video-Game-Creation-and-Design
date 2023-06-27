#ifndef _BT_CUPCAKE_EXPLOSIVE
#define _BT_CUPCAKE_EXPLOSIVE

#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "bt_controller.h"
#include "geometry/curve.h"
#include "modules/game/audio/audioEvent.h"

class CBTCupcake_explosive : public BTController {
	CHandle h_player;
	DECL_SIBLING_ACCESS();
public:
	void create(string s); //init
	void updateBT();
	 //acciones == hojas

	void setCurve(const CCurve* curve);

	int actionChangeWpt();
	int actionExplode();
	int actionSeekWpt();
	int actionAttack();
	int actionChangeState();


	int actionDeath();
	int actionGravityReceived();
	int actionImpactReceived();
	int actionRecoilReceived();


	//condiciones para establecer prioridades

	bool conditionExplode();
	bool conditionView();
	bool conditionWptClose();


	bool conditionDeath();
	bool conditionGravityReceived();
	bool conditionImpactReceived();
	bool conditionRecoilReceived();
	bool conditionTimer();


	void generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc);
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
	static void registerMsgs();

	template <typename T>
	void Send_DamageMessage(CEntity* entity, float dmg);

	void load(const json& j, TEntityParseContext& ctx);

	void movement(VEC3 target, bool seek);

	bool isGrounded();
	bool isView(float distance);
	bool checkHeight();

	int damage = 10;
	int fireDamage = 15;
	int currentDamage = damage;

	//CURVE  VALUES
	float	mTravelTime = 0.22f;
	float ratio = 0.f;
	const CCurve* _curve = nullptr;
	std::vector<VEC3> _knots;
	//END CURVE VALUES
	std::string getNameCurve();
private:
    AudioEvent voice;


	float height_range = 3.0f;
	CHandle _mySpawner;
	bool _initiallyPaused = false;

	//explosion values
	bool explosionActive = false;
	float distanceExplosion = 2.0;
	float explosionDelay = 2.0;
	float explosionTimer = explosionDelay;
	bool has_exploded = false;
	float explosion_damage = 25.0;
	float explosionRadius = 3.f;

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


	bool initialExecution = true;
	bool attacking = false;
	bool impacted = false;
	bool recoiled = false;
	bool insightPlayer = false;
	//gameplay parameters

	float life = 50.f;
	int spawnRange = 5; //spawn range of sons
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



	//Battery Values
	bool beingAttracted = false;
	float battery_time = 0.f;
	VEC3 battery_position = VEC3().Zero;
	VEC3 resultingForce = VEC3().Zero;
	float attractionForce = 60.f;
	float distanceToBattery = 0.f;
	float repulsionDuration = 0.4f;
	float repulsionTimer = 0.f;
	bool isDeadForFallout = false;


	//blackboard
	bool slotsAvailable = false;
	bool player_dead = false;
	bool inBlackboard = false;

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
	float jumpForce = 5.0f; //5
	float recoilJumpForce = 5.0f; //5

	//movement
	float distanceCheckThreshold = 1.0f;
	float forgetAttackingDistance = 20.f;
	//speeds
	float stunSpeed = 1.5f; //1.5
	float speed = 5.0f; //5
	float hideSpeed = 10.f; //10
	float twistSpeed = 340.f; //5 //radianes
	float batterySpeed = 20.f;

	float impulseStrenghtSecondSon = 7.f;

	float cone_vision = 14.f;
	float enemyRadiousView = 9.0f;
	bool view_point = false;
	float length_cone = 15.0f;
	float half_cone = 45.0f;
	std::string pathCurve = "";
	CHandle h_sender;
	CHandle h_bullet;
};



#endif _BT_CUPCAKE_EXPLOSIVE