#include "mcv_platform.h"
#include "engine.h"
#include "comp_madness_puddle.h"
#include "components/powers/comp_madness.h"


DECL_OBJ_MANAGER("comp_madness_puddle", TCompMadnessPuddle);

void TCompMadnessPuddle::debugInMenu() {
}

void TCompMadnessPuddle::load(const json& j, TEntityParseContext& ctx) {

}

void TCompMadnessPuddle::registerMsgs() {
	DECL_MSG(TCompMadnessPuddle, TMsgDamage, onPlayerAttack);
  DECL_MSG(TCompMadnessPuddle, TMsgEntityCreated, onCreation);
}

void TCompMadnessPuddle::onCreation(const TMsgEntityCreated& msg) {
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesBleach");
    buf->updateGPU(&constants);
  }
}

void TCompMadnessPuddle::onPlayerAttack(const TMsgDamage & msg) {
	dbg("Madness puddle cleansed\n");
	CEntity* p = msg.h_sender;
	TCompMadnessController* m_c = p->get<TCompMadnessController>();
	if(m_c != nullptr) {
		m_c->generateMadness(PowerType::PUDDLE);
		GameController.addMadnessPuddle();
		//GameController.healPlayerPartially(10.f); //curar al player
		EngineAudio.playEvent("event:/Character/Attacks/Clean_Puddle");
    enabled = true;
	}
}

void TCompMadnessPuddle::update(float dt) {

  ratio = clamp(ratio - 4.0f*dt*enabled, 0.0f, 1.0f);
  float size = Interpolator::quartOut(0.f, 1.f, ratio);
  constants.x = size;
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesBleach"); 
    buf->updateGPU(&constants);
  }

  if (size <= 0.0f) {
    CHandle(this).getOwner().destroy();
    CHandle(this).destroy();
  }
}
