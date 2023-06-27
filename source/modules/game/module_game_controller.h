#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "datatypes.h"
#include "geometry/curve.h"
#include "components/controllers/comp_curve_controller.h"
#include "skeleton/comp_skel_lookat.h"
#include "modules/game/audio/audioEvent.h"
#include "components/animation/comp_morph_animation.h"
#include "components/controllers/comp_enemies_in_tube.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_camera.h"
#include "components/objects/comp_enemy_spawner.h"
#include "components/ai/others/self_destroy.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/common/comp_light_point.h"
#include "components/vfx/comp_flickering.h"
#include "components/objects/comp_balance.h"
#include "components/common/comp_light_dir.h"
#include "ui/ui_widget.h"


class CBTCupcake;
class CBTGolem;
class CCheckpoint;
class CModuleGameplayFragment;

class CModuleGameController : public IModule
{
    /* Mantain a handle of the player */
    CHandle _player;
    CHandle _fly_camera;
    //Storage of entity status
    CCheckpoint* _lastCheckpoint;
    bool god_mode = false;
	bool invisible_block_cursor = false;
	bool resurrect = false;

    CModuleGameplayFragment* gf;

    // Menu window related variables.
    ImGuiWindowFlags window_flags;
    unsigned int window_width;
    unsigned int window_height;
    unsigned int menuPosition;
    const unsigned int menuSize = 4;
	//Im	Gui Bar size
	float hp_bar_size = 250.f;
	float madness_bar_size = 250.f;
	float extraBarSize = 75.f;

    void resetState();
    void renderInMenu();
    void updateGameCondition();

	std::vector<VEC3> positionAreas{VEC3(414.479,-26.2468,-49.352),VEC3(54,2,-55),VEC3(176,4 ,-17),VEC3(2,7 ,-1),VEC3(14, 15,-6),VEC3(-69,2,-114),VEC3(-83,5,-212), VEC3(-122,46,-223) };
	//std::vector<VEC3> positionAreas{ VEC3(40, 9,-4) };
	int positionCheat = 0;
    
    //void switchState(PauseState pause);

	//Madness Puddles
	int _madnessPuddlesCleansed = 0;
	//End Madness Puddles

	//EnemiesKilled
	int enemiesKilled = 0;
	//End EnemiesKilled
public:
#define PREFAB_SUSHI "data/prefabs/enemies/bt_sushi.json"
#define PREFAB_RANGED_SUSHI "data/prefabs/enemies/bt_ranged_sushi.json"
#define PREFAB_CUPCAKE "data/prefabs/enemies/bt_cupcake.json"
#define PREFAB_GOLEM "data/prefabs/enemies/golem.json"
#define PREFAB_TELEPORTABLE_OBJECT "data/prefabs/structures/teleport_box.json"

    const std::map<EntityType, std::string> _prefabs {
        {SUSHI, "data/prefabs/enemies/bt_sushi.json"},
        {RANGED_SUSHI, "data/prefabs/enemies/bt_ranged_sushi.json"},
        {CUPCAKE, "data/prefabs/enemies/bt_cupcake.json"},
        {GOLEM, "data/prefabs/enemies/golem.json"},
        {CUPCAKE_SPAWNER, "data/prefabs/structures/enemy_spawner.json"},
        {EXPLOSIVE_OBJECT, "data/prefabs/structures/explosive_object.json"},
        {DUMPLING_TURRET, "data/prefabs/structures/spawner_dumpling.json"},
        {THERMOSTATE, "data/prefabs/structures/termostate.json"}
    };

#define TYPE_SUSHI 1
#define TYPE_RANGED_SUSHI 2
#define TYPE_CUPCAKE 3 
#define TYPE_GOLEM 4 

  float yaw_sensivity = 5.f;
  float pitch_sensivity = 2.f;

	//enum PauseState { none, main, win, defeat, editor1, editor1unpaused, editor2, void_state };
    //PauseState _currentstate;

	CModuleGameController() : IModule("KK") {};
	CModuleGameController(const std::string& name) : IModule(name) {}

	CModuleGameController* getPointer() { return this; }


	//CModuleGameController();

    bool start() override;
    void update(float delta) override;
    void stop() override;
    //PauseState getCurrentState();
	  CHandle getPlayerHandle();
    bool getGodMode() { return god_mode; }
	bool getResurrect() { return resurrect; }
	void setGodMode(bool _god_mode);
	void setResurrect(bool _resurrect);
	bool getInvisibleBlock() { return invisible_block_cursor; }

	//Checkpoints
    bool saveCheckpoint(VEC3 playerPos, QUAT playerRot);
    bool loadCheckpoint();
    bool deleteCheckpoint();
    bool isCheckpointSaved();
	PowerType getPowerSelected();
	void savePower(PowerType power);
    void wakeUpWinds();
	//End Checkpoints

	//Entity handling
    CHandle spawnPrefab(const std::string prefab, Vector3 pos, QUAT rot = QUAT().Identity, float scale = 1.0f);
    CHandle spawnPuddle(Vector3 pos, QUAT rot = QUAT().Identity, float scale = 1.0f);
	void deleteByTag(const char* tag);
	//End Entity handling

	//Behavior Tree Control
  void stopStateMachines();
  void stopBehaviorTrees();
	void resumeBehaviorTrees();
	//End Behavior Tree Control

	//Time Control
	void stopTime();
	void resumeTime();
	void setTimeScale(float timeScale);
	float getTimeScale();
	//End Time Control

	//Madness Puddles
	void addMadnessPuddle();
	int getMadnessPuddles();
	//End Madness Puddles

	//EnemiesKilled
	void addEnemiesKilled(EntityType e);
	int getEnemiesKilled();
	//End EnemiesKilled

	//get and set bar
	float getHpBarSize(); 
	void increaseHpBarSize(float _madness_bar_size);
	float getMadnessBarSize();
	void increaseMadnessBarSize(float _madness_bar_size);

	//player
  void  healPlayer();
  void  healPlayerPartially(float health);
	void restoreMadness();
  void blendPlayerCamera();

	//Damage Generators
	void generateDamageSphere(VEC3 center, float radius, TMsgDamage message, const char* targetFilter);
	//End Damage Generators

	void addPowerUp(float quantity, PowerUpType type);
	int getNumThermoStatesActives();
	void activateSecondEvent();
  //scripting situations and checkpoint helper
  void activateGameplayFragment(std::string module);
  void deactivateGameplayFragment(std::string module);
  //scripting situations and checkpoint helper

  //lua functions
  void wakeUpSushi(std::string id);
  void deleteElement(std::string id);
  void activateSpawner(std::string id);
  //lua functions
  void spawnPrefabByPoint(const std::string prefab, const std::string name_origin);
  //setear damage
  void setDamage(const std::string name_origin, int damage );
  //bindear entidades en curvas
  void bindInCurve(std::string _curve, std::string entity);
  //blending camaras con interpolador
  void blendingCamera(std::string camDest,float blendTime, std::string typeInterpolation);
  //activar animacion de morph
  void playAnimationMorph(std::string name);
  //desactivar animacion de morph
  void stopAnimationMorph(std::string name);

  //bloquear camera principal
  void lockCamera3Person(bool activate);
  //resetCamera
  void resetCamera();
  //activar modo cinematica en jugador
  void inCinematic(bool active);
  //activar modo cinematica special
  void inCinematicSpecial(bool active,int type);
  //activar modo cinematica en golem
  void inCinematicGolem(std::string name, bool active);
  //delete handle
  void destroyCHandleByName(std::string name);
  //activar plataformas
  void activatePlatformByName(std::string name);
  //spawnear y poner nombre a la entidad
  void toNamePrefab(const std::string name, CHandle handle);
  //cargar escena
  void pausedPlayer(bool value);
  void loadScene(const std::string name);
  void GPUloadScene(const std::string name);
  void GPUdeleteScene(const std::string name);
  template <typename T>
  void broadcastMessage(T message);

	void setPauseEnemyByName(std::string enemy, bool active);
	void setPauseEnemyByHandle(CHandle h_enemy, bool active);
	void updateEnemyCurveByName(std::string _curve, std::string enemy);
	void updateEnemyCurveByHandle(std::string _curve, CHandle h_enemy);
	void updateCupcakeCurveByHandle(std::string _curve, CHandle h_enemy);

	void updatePlatformCurveByName(std::string _curve, std::string name_platform);
	
	void setTransformObject(std::string name, VEC3 pos,float yaw, float pith, float roll);

	//destruir un muro
	void destroyWallByName(std::string name, std::string golemName, float intensity = 20);

  void deleteProducts();
  void loadProducts(std::string zona);

	//despertar golem
	void wakeUpGolem(std::string name);
	//dormir al golem
	void sleepGolem(std::string name);

    //Soundtrack Functions
    void startSoundtrack(int track_id);
    void pauseSoundtrack();
    void resumeSoundtrack();
    void updateSoundtrackID(int new_track_id);
    void updateAmbientLight(float amount);
    void setSoundtrackVolume(float volume);
    float getSoundtrackVolume();
    void playAnnouncement(std::string announcement);
    void startAudioPlayer(std::string entity);
    //End Soundtrack Functions
	//dar handle a partir del nombre 
	CHandle entityByName(std::string name);
	void cheatPosition();

    void pauseGame();
    void resumeGame();

	void dbgInLua(std::string text);
	void setHeightEnemyByHandle(int height, CHandle h_enemy, int typeEnemy);
	void setViewDistanceEnemyByHandle(float distance, CHandle h_enemy, int typeEnemy);
	void setHalfConeEnemyByHandle(float half_cone, CHandle h_enemy, int typeEnemy);
	CCamera* getCameraFromHandle(CHandle hCamera);
	void deleteCupcake();
	void deleteSushi();
	void deleteGolem(std::string name);
	void setLifeEnemy(CHandle h, int typeEnemy, float life);
	void setLifeEnemiesByTag(const char* tagName, float life);

	void changeGameState(std::string name);
	void deactivateWidget(std::string name);
	void activateWidget(std::string name);
	void childAppears(std::string name, bool getFromChildren, bool alfaPos, float valueIni, float valueFin);


	void changeShadowsEnabledJoint(bool value);
	void changeLightsIntensityJoint(float value);

	void stopWidgetEffect(const std::string& nameWidgetStrMap, const std::string& nameEffect);
	void changeSpeedWidgetEffect(const std::string& nameWidgetStrMap, const std::string& nameEffect, float x, float y);
	void stopWidgetEffectSpecial();
	void changeSpeedWidgetEffectSpecial(float x, float y);
	void changeDurationWidgetEffectSpecial(float duration);
	void resurrectionInGameOver();
	void exitGame();
	void setBloomInCam(bool value);
};


//aux function

TCompCurveController* TCurveController(CHandle h);
CEntity* toEntity(CHandle h);
TCompSkelLookAt* toCompSkelLookAt(CHandle h);
TCompMorphAnimation* toCompMorphAnimation(CHandle h);
TCompEnemiesInTube* toCompEnemiesInTube(CHandle h);
TCompName* toCompName(CHandle h);
TCompTransform* toCompTransform(CHandle h);
TCompCamera* toCompCamera(CHandle h);
//TCompEnemySpawnerSpecialTrap* toCompEnemySpawnerSpecialTrap(CHandle h);
TCompSelfDestroy* toCompSelfDestroy(CHandle h);
CBTGolem* toCBTGolem(CHandle h);
CBTCupcake* toCBTCupcake(CHandle h);
TCompEnemySpawner* toCompEnemySpawner(CHandle h);
TCompLightPoint* toCompLightPoint(CHandle h);
TCompFlickering* toCompFlickering(CHandle h);
TCompCharacterController* toCompCharacterController_(CHandle h);
TCompBalance* toCompBalance(CHandle h);
TCompLightDir* toCompLightDir(CHandle h);
