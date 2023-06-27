#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/physics/comp_collider.h"
#include "modules/game/audio/audioEvent.h"

class TCompAudioPlayer : public TCompBase {

	
	DECL_SIBLING_ACCESS();

    void onSoundRequest(const TMsgSoundRequest& msg);
public:
    void play();
    void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	static void registerMsgs();

    /*void play();
    void stop();
    void pause();
    void resume();
    void restart();*/

    AudioEvent getEvent();

private:
    std::string _audioEventString = "";
    std::string _audioEventStringSlowVersion = "";
	bool _autoPlay = false;
    AudioEvent _audioEvent;
    AudioEvent _audioEventSlow;
};