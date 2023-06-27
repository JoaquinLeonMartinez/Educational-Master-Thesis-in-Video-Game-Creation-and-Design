#ifndef INC_COMMON_ENTITY_MSGS_
#define INC_COMMON_ENTITY_MSGS_

#include "geometry/transform.h"
#include "msgs.h"
#include "datatypes.h"

struct TMsgEntityCreated {
  DECL_MSG_ID();
};

struct TMsgOnomPet {
  VEC3 pos;
  float type;
  DECL_MSG_ID();
};

struct TMsgDamage {
	CHandle h_sender;
	CHandle h_bullet;
	float intensityDamage;
	float impactForce = 0.f;
	VEC3 bullet_front;
	PowerType damageType;
	EntityType senderType;
	EntityType targetType;
	VEC3 position;
	DECL_MSG_ID();
};

struct TMsgTrapWind {
  CHandle h_sender;
  CHandle h_bullet;
  float intensityDamage;
  float impactForce = 0.f;
  VEC3 bullet_front;
  PowerType damageType;
  EntityType senderType;
  EntityType targetType;
  VEC3 position;
  DECL_MSG_ID();
};

struct TMsgAssignBulletOwner {
	CHandle h_owner;
	CHandle h_target;
	VEC3    source;
	VEC3    targetPoint;
  VEC3    front;
  TMsgDamage messageToTarget;
  DECL_MSG_ID();
};

struct TMsgToCupcakeSon {
	//CHandle h_owner;
	//VEC3    source;
	float life;
	int num_of_sons;
	bool insightPlayer;
	bool attacking;
  float damage;
	VEC3 impulse;
	DECL_MSG_ID();
};

struct TMsgOnContact {
  CHandle source;
  VEC3 pos;
  DECL_MSG_ID();
};

struct TMsgBatteryDeactivates {
    DECL_MSG_ID();
};

struct TMsgBTPaused {
  bool isPaused;
  DECL_MSG_ID();
};

struct TMsgAIPaused {
    bool isPaused;
    DECL_MSG_ID();
};

struct TMsgPowerUp {
  int power_type;
  DECL_MSG_ID();
};

struct TMsgMunition {
	int power_type;
	int amount;
	DECL_MSG_ID();
};

struct TMsgOnRemove {
  CHandle target;
  DECL_MSG_ID();
};

//Send this message to the spawned enemy for it to know who spawned it
struct TMsgSpawnerCheckin {
	CHandle spawnerHandle;
	DECL_MSG_ID();
};

struct TMsgSpawnerFather {
	CHandle son;
	DECL_MSG_ID();
};

struct TMsgActiveGolem {
	bool active;
	DECL_MSG_ID();
};


//Send this message to the spawner for it to know that the enemy has died
struct TMsgSpawnerCheckout {
	CHandle enemyHandle;
	DECL_MSG_ID();
};

//parar pj
struct TMsgOnCinematic {
	bool cinematic;
	bool isscart;
	DECL_MSG_ID();
};

struct TMsgOnCinematicSpecial {
	bool cinematic;
	bool isscart;
	int type;
	DECL_MSG_ID();
};


/*
struct TMsgDamageToEnemy {
CHandle h_sender;
CHandle h_bullet;
float intensityDamage;
VEC3 bullet_front;
PowerType damageType;
DECL_MSG_ID();
};
*/


struct TMsgDamageToAll {
	float intensityDamage;
	DECL_MSG_ID();
};

/*
struct TMsgDamageToPlayer {
CHandle h_sender;
CHandle h_bullet;
float intensityDamage;
VEC3 bullet_front;
//falta el owner y el da�o elemental
DECL_MSG_ID();
};
*/

struct TMsgGravity {
	CHandle h_sender;
	CHandle h_bullet;
	VEC3 position;
	VEC3 bullet_front;
	float time_effect;
	float distance;
	float attractionForce;

	DECL_MSG_ID();
};


struct TMsgEntityTriggerEnter {
	CHandle h_entity;
	DECL_MSG_ID();
};

struct TMsgEntityTriggerStay {
  CHandle h_entity;
  DECL_MSG_ID();
};

struct TMsgEntityTriggerExit {
	CHandle h_entity;
	DECL_MSG_ID();
};


struct TMsgFireAreaEnter {
	CHandle h_entity;
	VEC3 areaCenter;
	DECL_MSG_ID();
};

struct TMsgFireAreaExit {
	CHandle h_entity;
	DECL_MSG_ID();
};


// Sent to all entities from a parsed file once all the entities
// in that file has been created. Used to link entities between them
struct TEntityParseContext;
struct TMsgEntitiesGroupCreated {
  const TEntityParseContext& ctx;
  DECL_MSG_ID();
};

struct TMsgDefineLocalAABB {
  AABB* aabb = nullptr;
  DECL_MSG_ID();
};

struct TMsgSetVisible {
  bool visible;
  DECL_MSG_ID();
};

struct TMsgSetActive {
	bool active;
	DECL_MSG_ID();
};

struct TMsgBlackboard {
	CHandle h_entity; //para cuando el enemigo pide permiso
	bool player_dead;
	bool is_posible; //para la respuesta
	EntityType type;
	DECL_MSG_ID();
};

struct TMSgWallDestroyed {
	CHandle h_entity;
	bool isDetroyed;
	DECL_MSG_ID();
};

struct TMSgEnemyDead {
	CHandle h_entity;
	std::string name;
	bool isDead;
	DECL_MSG_ID();
};

struct TMSgTriggerFalloutDead {
	CHandle h_entity;
	bool falloutDead;
	float damage;
	DECL_MSG_ID();
};

struct TMsgGamePause {
	bool isPause;
	bool playerDead;
	DECL_MSG_ID();
};

struct TMsgSoundRequest {
    std::string name;
    DECL_MSG_ID();
};

struct TMsgDeleteTrigger {
	CHandle h_entity;
	bool deleteForTrigger;
	float damage;
	DECL_MSG_ID();
};

struct TMsgMeleeHit {
  CHandle h_entity;
  DECL_MSG_ID();
};

/*
struct TMsgTimeOut {
  DECL_MSG_ID();
};

struct TMsgSpawn {
  DECL_MSG_ID();
};

struct TMsgSpawnAt {
  CTransform where;
  DECL_MSG_ID();
};
*/

#endif

