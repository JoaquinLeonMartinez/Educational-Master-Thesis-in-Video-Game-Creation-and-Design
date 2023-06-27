#include "mcv_platform.h"
#include "engine.h"
#include "comp_restore_madness.h"
#include "components/powers/comp_madness.h"


DECL_OBJ_MANAGER("comp_restore_madness", TCompRestoreMadness);

void TCompRestoreMadness::debugInMenu() {
}

void TCompRestoreMadness::load(const json& j, TEntityParseContext& ctx) {

	madness = j.value("madness", madness);
}

void TCompRestoreMadness::registerMsgs() {
	DECL_MSG(TCompRestoreMadness, TMsgDamage, onPlayerAttack);
}

void TCompRestoreMadness::onPlayerAttack(const TMsgDamage & msg) {
	
	CEntity* p = msg.h_sender;
  if (p) {
    TCompMadnessController* m_c = p->get<TCompMadnessController>();
    if (m_c != nullptr) {
      m_c->generateMadness(madness);
    }
  }
}

void TCompRestoreMadness::update(float dt) {

}
