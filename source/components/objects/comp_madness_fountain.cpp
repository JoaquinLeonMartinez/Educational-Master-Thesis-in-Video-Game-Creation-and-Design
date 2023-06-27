#include "mcv_platform.h"
#include "engine.h"
#include "comp_madness_fountain.h"
#include "components/powers/comp_madness.h"


DECL_OBJ_MANAGER("comp_madness_fountain", TCompMadnessFountain);

void TCompMadnessFountain::debugInMenu() {
}

void TCompMadnessFountain::load(const json& j, TEntityParseContext& ctx) {
    audio = EngineAudio.playEvent("event:/Music/Ambience_Props/Madness_Fountain/Madness_Fountain_Loop");
    audioSlow = EngineAudio.playEvent("event:/Music/Ambience_Props/Madness_Fountain/Madness_Fountain_Loop_Slow");
    audioSlow.setPaused(true);
}

void TCompMadnessFountain::registerMsgs() {
	DECL_MSG(TCompMadnessFountain, TMsgEntityTriggerEnter, enable);
	DECL_MSG(TCompMadnessFountain, TMsgEntityTriggerExit, disable);
}

void TCompMadnessFountain::enable(const TMsgEntityTriggerEnter & msg) {
	if (msg.h_entity == GameController.getPlayerHandle()) {
		_isEnabled = true;
	}
}

void TCompMadnessFountain::disable(const TMsgEntityTriggerExit & msg) {
	if (msg.h_entity == GameController.getPlayerHandle()) {
		_isEnabled = false;
	}
}

void TCompMadnessFountain::update(float dt) {
	if (_isEnabled) {
		CEntity* p = GameController.getPlayerHandle();
		TCompMadnessController* m_c = p->get<TCompMadnessController>();
        float value = m_c->getPowerGeneration(PowerType::FOUNTAIN) * dt;
		    m_c->generateMadness(value);
        GameController.healPlayerPartially(value);
	}
    TCompTransform* c_trans = get<TCompTransform>();
    audioSlow.set3DAttributes(*c_trans);
    audio.set3DAttributes(*c_trans);

    if (GameController.getTimeScale() < 1.0f && audioSlow.getPaused()) {
        audioSlow.setPaused(false);
        audio.setPaused(true);
    }
    else if (GameController.getTimeScale() == 1.0f && audio.getPaused()) {
        audio.setPaused(false);
        audioSlow.setPaused(true);
    }
}
