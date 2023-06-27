#include "mcv_platform.h"
#include "comp_morph_animation.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_buffers.h"


DECL_OBJ_MANAGER("morph_animation", TCompMorphAnimation);

void TCompMorphAnimation::debugInMenu() {
	ImGui::Checkbox("Play", &play_);

	if (ImGui::DragFloat4("Morph Weights", &morph_weights.a, 0.01f, 0.0, 1.0f)) {
	  TCompBuffers* c_buff = get<TCompBuffers>();
	  if (c_buff) {
		  auto buf = c_buff->getCteByName("TCtesMorph");
		  buf->updateGPU(&morph_weights);
	  }
  }
}

void TCompMorphAnimation::load(const json& j, TEntityParseContext& ctx)
{
	
	increment = j.value("increment", increment);
	play_ = j.value("play", play_);
}

void TCompMorphAnimation::renderDebug()
{
  
}

void TCompMorphAnimation::update(float dt)
{
	if (play_) {
		TCompBuffers* c_buff = get<TCompBuffers>();
		if (c_buff) {
			auto buf = c_buff->getCteByName("TCtesMorph");
			//buf->updateGPU(&dt);
			if (morph_weights.a <= 1.0) {
				morph_weights.a += increment * dt;
      }
      else {
        CHandle(this).getOwner().destroy();
        CHandle(this).destroy();
      }
			buf->updateGPU(&morph_weights);
		}
	}
}

void TCompMorphAnimation::playMorph() {
	play_ = true;
}

void TCompMorphAnimation::stopMorph() {
	play_ = false;
}

void TCompMorphAnimation::setIncrement(float i) {
  increment = i;
}

void TCompMorphAnimation::updateMorphData(float amount) {
  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesMorph");
    //buf->updateGPU(&dt);
    morph_weights.a = amount;
    buf->updateGPU(&morph_weights);
  }
}

float TCompMorphAnimation::getWeight() {
  return morph_weights.a;
}