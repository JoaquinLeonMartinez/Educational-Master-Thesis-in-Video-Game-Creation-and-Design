#pragma once

#include "components/ai/bt/bt_controller.h"
#include "datatypes.h"

struct EnemyStatus {
    std::string enemyName;
    std::string enemyIAStateName;
	  EntityType enemyType;
    VEC3 enemyPosition;
    QUAT enemyRotation;
    bool saved = false;
};

struct PlayerStatus {
    VEC3 playerPos;
    QUAT playerRot;
	bool teleport = true;
	bool battery = false;
	bool chilli = false;
	bool coffe = false;
    bool saved = false;
	PowerType power_selected;
};

struct EntityStatus {
    std::string entityName = "";
    std::string entityPrefab = "";
    EntityType entityType = UNDEFINED;
    VEC3 entityPos = VEC3().Zero;
    QUAT entityRot = QUAT().Identity;
    CHandle handle;
    bool saved = false;
	string curve = "";
	float lifeCupcake = 50.f;
	int comportamentNormal = 0;
	bool ovenDestroyed = false;
	float pauseBT;
	
};

class CCheckpoint {

public:

    CCheckpoint();

    //bool init();
    bool saveCheckPoint(VEC3 playerPos, QUAT playerRotation);
    bool loadCheckPoint();
    bool deleteCheckPoint();
    const bool isSaved() { return saved; };
    void debugInMenu();
	PlayerStatus getPlayerStatus();
	bool savePower(PowerType power);
private:

    PlayerStatus playerStatus;
    std::vector<EnemyStatus> enemies;
    std::vector<EntityStatus> entities;
    bool saved;

};