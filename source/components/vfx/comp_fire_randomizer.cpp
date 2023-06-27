#include "mcv_platform.h"
#include "comp_fire_randomizer.h"
#include "components/common/comp_buffers.h"
#include "components/common/comp_light_point.h"
#include "random"

std::mt19937 bt_firerand(1992);
std::uniform_int_distribution<int> bt_range5(0, 100);

DECL_OBJ_MANAGER("fire_randomizer", TCompFireRandomizer);

void TCompFireRandomizer::debugInMenu() {
  
}

void TCompFireRandomizer::load(const json& j, TEntityParseContext& ctx) {
  
}



void TCompFireRandomizer::registerMsgs() {
  DECL_MSG(TCompFireRandomizer, TMsgEntityCreated, onCreation);
}

void TCompFireRandomizer::onCreation(const TMsgEntityCreated& msgC) {
  float phase = bt_range5(bt_firerand) / 100.0f;
  constants.x = phase;
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesFire");
    buf->updateGPU(&constants);
  }
}

void TCompFireRandomizer::renderDebug() {
}

void TCompFireRandomizer::update(float delta) {
  
}


