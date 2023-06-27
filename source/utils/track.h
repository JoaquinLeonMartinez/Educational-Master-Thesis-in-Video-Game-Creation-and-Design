#pragma once

#include "mcv_platform.h"

template <typename T>
bool renderKeyFrameInMenu(T& v);

template <typename VAL>
struct TTrack {
private:
  struct TKeyframe {
    float time;
    VAL   value;
  };

  std::vector<TKeyframe>  keyframes;
  bool                    sorted;
  VAL                     defaultValue;

public:
  TTrack() : sorted(false) { }

  void setDefault(VAL value)
  {
    defaultValue = value;
  }

  void set(float t, VAL value) {
    TKeyframe k;
    k.time = t;
    k.value = value;
    keyframes.push_back(k);
    sorted = false;
  }

  VAL get(float t) const {
    /*if (!sorted)
      sort();*/

    if (keyframes.empty()) {
      return defaultValue;
    }
    else if (keyframes.size() == 1) {
      return keyframes[0].value;
    }
    else {
      TKeyframe curr = keyframes[0];
      for (unsigned i = 1; i < keyframes.size(); ++i) {
        const TKeyframe& next = keyframes[i];
        if (next.time > t) {
          float t_ratio = (t - curr.time) / (next.time - curr.time);
          VAL v_ratio = curr.value + (next.value - curr.value) * t_ratio;
          return v_ratio;
        }
        else {
          curr = next;
        }
      }
      return curr.value;
    }
  }

  void sort() {
    auto sorter = [](const TKeyframe& k1, const TKeyframe& k2) {
      return k1.time < k2.time;
    };
    std::sort(keyframes.begin(), keyframes.end(), sorter);
    sorted = true;
  }

  std::vector<TKeyframe>& getKeyframes() {
    return keyframes;
  }

  void uniformSample(uint32_t num_samples, VAL* samples, float t0 = 0.0f, float t1 = 1.0f) const {
    assert(num_samples > 1);
    for (uint32_t i = 0; i < num_samples; ++i) {
      float ut = (float)i / (float)(num_samples - 1);
      float t = t0 + (t1 - t0) * ut;
      samples[i] = get(t);
    }
  }

  bool renderInMenu( const char* title ) {
    bool changed = false;
    if (ImGui::TreeNode(title)) {
      for (int i = 0; i < keyframes.size(); ++i) {
        TKeyframe& k = keyframes[i];
        ImGui::PushID(i);

        // Quick & dirty add/del keys
        if (ImGui::SmallButton("-")) {
          changed = true;
          keyframes.erase(keyframes.begin() + i);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+")) {
          changed = true;
          TKeyframe nk;
          nk.time = 1.f;
          nk.value = defaultValue;
          keyframes.push_back(nk);
        }
        ImGui::SameLine();

        if (ImGui::DragFloat("Time", &k.time, 0.01f, 0.f, 1.0f))
          changed = true;
        changed |= renderKeyFrameInMenu<VAL>(k.value);
        ImGui::PopID();
      }
      ImGui::TreePop();
    }
    if (changed)
      sort();
    return changed;
  }

};
