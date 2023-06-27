#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "components/ai/graph/ai_controller.h"
#include "modules/game/audio/audioEvent.h"

class TCompSCartController : public IAIController
{
public:

	

	float life = 100.0f;
	std::string weapon_selected = "data/prefabs/bullets/bullet_bounce.json";

	bool aiming = false;
	bool is_grounded = false;
	void Init();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void onDamage(const TMsgDamage& msg);
	void SwapMesh(int state);

	static void registerMsgs();
	void renderDebug();
	//Shopping Cart Public Functions
	void enable(CHandle vehicle);
	void disable();
	//End Shopping Cart Public Functions
    bool cinematic = false;
	bool isEnabled = false;
private:
	CHandle vehiclePropHandle;
	CHandle fakePlayerHandle;
	//Flags
	
    //Audio Values
    AudioEvent _movementAudio;
    AudioEvent _crashAudio;
    //Audio Values
	
	float sensitivity = 2.0f;
	float gravity = -9.81f;
	float speed = 6.0f;
	float rowDelay = 1.0f;
	float rowTimer = 0.f;
	float rowImpulse = 15.f;
  float inclinationRow = 8.f;
	float rowImpulseLeft = 0.f;
	float rowImpulseLossRatio = 5.0f;
	float rotation_speed = 1.3f;
  float maxImpulse = 40.0f;

	float rumble_time = 0.0f;

  VEC3 air_dir = VEC3();

	CHandle h_camera;

	bool gravity_enabled = true;

	//Shopping Cart States
    void disabled();
    void idleCinematic();
	void rowing(float delta);
	//End Shopping Cart States

	void grounded(float delta);
	void onAir(float delta);
	void damaged(float delta);
	void dead(float delta);
	void treatRumble(float delta);
	void rotatePlayer(float delta);
	bool isGrounded();
  void expansiveWave();

	void onCollision(const TMsgOnContact& msg);
    void onCinematicScart(const TMsgOnCinematic& msg);
	void onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg);

	

	DECL_SIBLING_ACCESS();
};

