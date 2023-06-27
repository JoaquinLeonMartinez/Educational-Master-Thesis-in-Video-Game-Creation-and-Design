#ifndef _AI_CUPCAKE
#define _AI_CUPCAKE

#include "ai_controller.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"


class CAICupcake : public IAIController
{
	//active states
	void SeekwptState(float dt);
	void NextwptState(float dt);
	void ChaseState(float dt);
	void OrbitState(float dt);
	void NextOrbitState(float dt);
	void AlignState(float dt);


	//pasive states
	void ImpactState(float dt);
	void RecoilState(float dt);
	void DeadState(float dt);
	void DivideState(float dt);
	void RecoilState2(float dt);
	void FallState(float dt);
	void BatteryEfectState(float dt);

	//other
	void generatePoints();
	bool isView(float distance);
	bool isGrounded();
	void movement(float dt, VEC3 target, bool seek);

	void onCollision(const TMsgOnContact& msg);
	void onDamage(const TMsgDamageToEnemy& msg);
	void onGravity(const TMsgGravity& msg);
	//void onBorn(const TMsgCupcakeSon& msg);


	CHandle h_player;

	DECL_SIBLING_ACCESS();
public:


	void Init();
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	void renderDebug();
	static void registerMsgs();


	CHandle h_sender;
	CHandle h_bullet;

	//damages
	int damage = 10; //20

	int id;

private:
	//list of way points
	bool init = true;
	std::vector<VEC3> positions;
	int wtpIndex = 0;
	VEC3 nextPoint;

	//list os orbit points
	std::vector<VEC3> orbitPositions;
	int orbitIndex = 0;
	VEC3 nextOrbitPoint;

	//timers
	float limitTimeStun = 0.1f;
	float timerStun = limitTimeStun;


	//speeds
	float stunSpeed = 1.5f; //1.5
	float speed = 5.0f; //5
	float hideSpeed = 10.f; //10
	float twistSpeed = 5.f; //5
	float batterySpeed = 20.f;

	//external effects
	float battery_time = 0.f;
	float distanceToBattery;
	VEC3 bullet_position = VEC3();
	VEC3 bullet_front = VEC3();
	VEC3 battery_position = VEC3();
	float bullet_impulse = 60.0; //15 deberia venir en el mensaje de la bala

	//external effects
	bool beingAttracted = false;

	//jump
	float gravity = -9.81f;
	VEC3 impulse = VEC3();
	bool firstImpact = true;
	float jumpForce = 5.0f; //5
	float recoilJumpForce = 5.0f; //5


								  //probabilities
	float attackProbability = 0.f; //attack probability when detect the player
	float orbitAttackProb; //attack probability while orbit

						   //ranges of vision
	float length_cone = 15.0f;
	float half_cone = 30.0f;
	float hearing_radius = 2.0f;
	float wptDistance = 4.0f; //accuracy 
	float cone_vision = 10.f; //para empezar a saltar al siguiente wtp
	bool view_point = false;

	float forgetEnemyRange = 1.0;
	float chase2Seekwtp = length_cone + forgetEnemyRange;
	float seewWtp2Chase = length_cone;

	//gameplay parameters
	int num_of_divisions; //times will you divide your self
	float life = 10.f;
	int num_of_sons = 2; //number of sons
	int spawnRange = 5; //spawn range of sons
	float orbitRadious = 7.0f;
	int minDistanceOrbit = 4.0; //for random
	int number_of_orbit_points = 4; //for random
	bool jumpingCupcake; //type os cupcake jumping or hide
	bool jump = true;
	bool hitting = true; //true if you are hitting , false if you have been hit
};

#endif _AI_CUPCAKE