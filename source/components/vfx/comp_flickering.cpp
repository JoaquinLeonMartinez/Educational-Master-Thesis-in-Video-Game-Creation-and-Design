#include "mcv_platform.h"
#include "engine.h"
#include "comp_flickering.h"
#include "components/common/comp_buffers.h"
#include "components/common/comp_light_point.h"
#include "components/common/comp_light_dir.h"
#include "random"

std::mt19937 bt_flick(1992);
std::uniform_int_distribution<int> bt_range2(0, 100);
std::uniform_int_distribution<int> bt_range3(0, 1);
std::uniform_int_distribution<int> bt_range50(0, 2);

DECL_OBJ_MANAGER("flickering", TCompFlickering);

void TCompFlickering::debugInMenu() {
  ImGui::DragFloat("Frequency", &frequency, 0.1f, 0.0f, 5.0f);
  ImGui::DragFloat("Amplitude", &amplitude, 0.1f, 0.0f, 5.0f);
  ImGui::DragFloat("Base", &base, 0.1f, 0.0f, 5.0f);
  ImGui::DragFloat("Phase ", &phase, 0.1f, 0.0f, 5.0f);
  ImGui::DragFloat("Phase 2 ", &phase2, 0.1f, 0.0f, 5.0f);
}

void TCompFlickering::load(const json& j, TEntityParseContext& ctx) {
  target = j.value("target", target);
  frequency = j.value("frequency", frequency);
  amplitude = j.value("amplitude", amplitude);
  base = j.value("base", base);
  phase = bt_range2(bt_flick); // start point inside on wave cycle
  phase2 = bt_range50(bt_flick); // start point inside on wave cycle
  phase3 = bt_range3(bt_flick); // start point inside on wave cycle

  if (strcmp(target.c_str(), "dir") == 0) {
      audio = EngineAudio.playEvent("event:/Music/Ambience_Props/Fluorescent_Loop");
  }
}

void TCompFlickering::renderDebug() {
}

void TCompFlickering::update(float delta) {
  if (strcmp(target.c_str(), "point") == 0) {
    float randomizer = bt_range3(bt_flick);
    float x = ((float)Time.current) * frequency * randomizer;
    float y;

    x = x - floor(x); // normalized value (0..1)

    y = sin(((x + phase / 100.0f) * 2 * PI));

    float randomizer2 = bt_range3(bt_flick);
    float res = (y * amplitude * randomizer) + base;

    TCompLightPoint* c_lp = get<TCompLightPoint>();
    c_lp->setIntensity(res);
  } else if (strcmp(target.c_str(), "dir") == 0) {
      TCompTransform* c_trans = get<TCompTransform>();
      if (c_trans)
        audio.set3DAttributes(*c_trans);
    float res;
    if (timer <= (frequency * phase2) + (base * phase3)) { // full light
      timer += delta;
      res = 1.0f * amplitude;
    }
    else if(timer <= (frequency * phase2) + (base * phase3) + 1.0f) { //the randomizer will make it flicker
        AudioEvent flicker = EngineAudio.playEvent("event:/Music/Ambience_Props/Fluorescent_Flicker");
        flicker.set3DAttributes(*c_trans);
      timer += delta;
      float x = ((float)Time.current) * frequency;
      x = sin(x) * sin(2 * x);
      if (x <= 0.0f)
        x = 0.0f;
      res = x * amplitude;
    }
    else {
      res = 0.0f * amplitude;
      timer = 0.0f;
      phase2 = bt_range50(bt_flick);
      phase3 = bt_range3(bt_flick);
    }

    TCompLightDir* c_ld = get<TCompLightDir>();
    c_ld->setIntensity(res);
  }

}


void TCompFlickering::setFrequency(float frequency_) {
	frequency = frequency_;
}
void TCompFlickering::setBase(float base_) {
	base = base_;
}
void TCompFlickering::setAmplitud(float amplitude_) {
	amplitude = amplitude_;
}
void TCompFlickering::setPhase(float phase_) {
	phase = phase_;
}

