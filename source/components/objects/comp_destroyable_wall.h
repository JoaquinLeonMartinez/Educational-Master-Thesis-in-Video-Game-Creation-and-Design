#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "modules/game/audio/audioEvent.h"

class TCompDestroyableWall : public TCompBase
{

  #define ICE_WALL 0 //translacion
  #define DESTROYABLE_WALL = 1//caja que se rompe en trozos
  #define NORMAL_WALL = 2 //o caja
	#define TIME_ICE_WALL = 3 // MURO DE HIELO QUE TARDA EN DERRETIRSE
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();

private:
  void onPlayerAttack(const TMsgDamage & msg);
  int typeWall = ICE_WALL;
  float factor = 1.5f;
	float factor2 = 2.5f;
	float timer_ice_wall = 150.0;
  float last_fire_hit = 0.0f;
  float max_last_fire_hit = 0.25f;
  bool enemies_in_tube_deleted = false;
  AudioEvent iceAudio;
  float iceAudioTimer = 0.f;
  float iceAudioDelay = 4.5f;
};

