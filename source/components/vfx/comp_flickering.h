#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "modules/game/audio/audioEvent.h"

class TCompFlickering :  public TCompBase {
  DECL_SIBLING_ACCESS();

  std::string target = "point";
  VEC4 constants = VEC4::Zero;
  float frequency = 0.5; // cycle frequency per second
  float phase = 0.0; // start point inside on wave cycle
  float amplitude = 1.0; // amplitude of the wave
  float base = 1.0; //start
  float timer = 0.0f;
  float phase2 = 0.0f;
  float phase3 = 0.0f;
  AudioEvent audio;

  public:
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void update(float delta);

	void setFrequency(float frequency);
	void setBase(float base);
	void setAmplitud(float amplitude_);
	void setPhase(float phase_);
};