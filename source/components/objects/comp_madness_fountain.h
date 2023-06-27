#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "modules/game/audio/audioEvent.h"

class TCompMadnessFountain : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  static void registerMsgs();

private:
    AudioEvent audio;
    AudioEvent audioSlow;
	bool _isEnabled = false;
	void enable(const TMsgEntityTriggerEnter & msg);
	void disable(const TMsgEntityTriggerExit & msg);
};

