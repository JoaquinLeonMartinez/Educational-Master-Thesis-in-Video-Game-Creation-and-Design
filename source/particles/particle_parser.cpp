#pragma once

#include "mcv_platform.h"
#include "particles/particle_parser.h"
#include "render/textures/material.h"
#include "utils/track.h"

template<>
bool renderKeyFrameInMenu<VEC4>(VEC4& clr) {
  return ImGui::ColorEdit4("Color", &clr.x);
}

class CParticleEmiterResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "particles"; }
  const char* getName() const override {
    return "Particles";
  }
  IResource* create(const std::string& name) const override {
    particles::TEmitter* emitter = new particles::TEmitter;
    particles::CParser parser;
    const json jData = loadJson(name);
    bool is_ok = parser.loadEmitter(*emitter, jData);
    assert(is_ok);
    emitter->setNameAndType(name, this);
    return emitter;
  }
};

template<>
const CResourceType* getResourceTypeFor<particles::TEmitter>() {
  static CParticleEmiterResourceType resource_type;
  return &resource_type;
}

void from_json(const json& j, TTrack<VEC4>& colors) {
  const VEC4 defaultColor = loadColor(j, "color", VEC4::One);
  colors.setDefault(defaultColor);
  if (j.count("colors"))
  {
    for (auto& jColor : j["colors"])
    {
      const float ratio = jColor[0].get<float>();
      const VEC4 color = loadColor(jColor[1]);
      colors.set(ratio, color);
    }
    colors.sort();
  }
}

void from_json(const json& j, TTrack<float>& sizes) {
  const float defaultSize = j.value<float>("size", 1.f);
  sizes.setDefault(defaultSize);
  if (j.count("sizes"))
  {
    for (auto& jSize : j["sizes"])
    {
      const float ratio = jSize[0].get<float>();
      const float size = jSize[1].get<float>();
      sizes.set(ratio, size);
    }
    sizes.sort();
  }
}

// ----------------------------------------------------------
void from_json(const json& j, TCtesParticles& p) {
  p.emitter_time_between_spawns = j.value("time_between_spawns", 1.0f);
  p.emitter_num_particles_per_spawn = j.value("num_particles_per_spawn", 1);
  p.emitter_dir = loadVEC3(j, "emitter_dir", VEC3(0, 1, 0));
  p.emitter_center = loadVEC3(j, "pos", VEC3(0, 0, 0));
  p.emitter_center_radius = j.value("center_radius", 0.0f);
  p.emitter_dir_aperture = j.value("dir_aperture", 0.0f);
  p.emitter_speed = loadVEC2(j, "speed");
  p.emitter_duration = loadVEC2(j, "duration", VEC2(1.0f, 1.0f));

  // Read and sample the colors
  TTrack<VEC4> colors = j;
  colors.uniformSample(particles::TEmitter::max_colors, p.psystem_colors_over_time);

  // Read the scales as float, but store them as VEC4 (because of the hlsl access array)
  assert(particles::TEmitter::max_sizes == 8 || fatal("particles::TEmitter::max_sizes should match TCtesParticles::sizes[]\n"));
  TTrack<float> sizes = j;
  float scalarValues[particles::TEmitter::max_sizes];
  sizes.uniformSample(particles::TEmitter::max_sizes, scalarValues);
  for (int i = 0; i < particles::TEmitter::max_sizes; ++i)
    p.psystem_sizes_over_time[i] = Vector4::One * scalarValues[i];
}

template<>
bool debugCteInMenu<TCtesParticles>(TCtesParticles& d) {
  bool changed = false;
  changed |= ImGui::DragFloat("time_between_spawns", &d.emitter_time_between_spawns, 0.01f, 0.01f, 2.0f);
  changed |= ImGui::DragInt("num_particles_per_spawn", (int*)&d.emitter_num_particles_per_spawn, 0.1f, 1, 12);
  changed |= ImGui::DragFloat3("center", &d.emitter_center.x, 0.01f, -15.f, 15.f);
  changed |= ImGui::DragFloat3("dir", &d.emitter_dir.x, 0.01f, -15.f, 15.f);
  changed |= ImGui::DragFloat("center_radius", &d.emitter_center_radius, 0.01f, 0.0f, 50.0f);
  changed |= ImGui::DragFloat("dir_aperture", &d.emitter_dir_aperture, 0.01f, 0.0f, 2.0f);
  changed |= ImGui::DragFloat2("speed", &d.emitter_speed.x, 0.01f, 0.f, 15.f);
  changed |= ImGui::DragFloat2("duration", &d.emitter_duration.x, 0.01f, 0.1f, 15.f);

  if (ImGui::TreeNode("Colors...")) {
    for (int i = 0; i < particles::TEmitter::max_colors; ++i) {
      char title[64];
      sprintf(title, "%d", i);
      changed |= ImGui::ColorEdit4(title, &d.psystem_colors_over_time[i].x);
    }
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Sizes...")) {
    for (int i = 0; i < particles::TEmitter::max_sizes; ++i) {
      char title[64];
      sprintf(title, "%d", i);
      changed |= ImGui::DragFloat(title, &d.psystem_sizes_over_time[i].x);
    }
    ImGui::TreePop();
  }

  return changed;
}

namespace particles
{

  bool CParser::loadEmitter(TEmitter& emitter, const json& jData)
  {
    emitter.count = jData.value<unsigned>("count", emitter.count);
    emitter.maxCount = jData.value<unsigned>("max_count", emitter.maxCount);
    emitter.interval = jData.value<float>("interval", emitter.interval);
    emitter.gravityFactor = jData.value<float>("gravity_factor", emitter.gravityFactor);
    emitter.direction = loadVEC3(jData, "direction", emitter.direction);
    emitter.duration = loadRange(jData, "duration", emitter.duration);
    emitter.speed = loadRange(jData, "speed", emitter.speed);
    emitter.frameCount = jData.value<int>("frame_count", emitter.frameCount);
    emitter.initialFrame = jData.value<int>("initial_frame", emitter.initialFrame);
    emitter.frameTime = jData.value<float>("frame_time", emitter.frameTime);
    emitter.frameSize = loadVEC2(jData, "frame_size", emitter.frameSize);
    emitter.particles_type_idx = jData.value<int>("particles_type", 0);
    
    //
    if (jData.count("material")) {
      emitter.material = Resources.get(jData["material"])->as<CMaterial>();
    }
    else {
      // We need to create one, using defaults close to the particles
      CMaterial* material = new CMaterial();
      // Create a fake json... it might be easier to just delegate the job to the json
      json j = json::object();
      j["textures"]["albedo"] = jData["texture"];
      bool is_additive = jData.value<bool>("additive", false);
      std::string default_tech = is_additive ? "instances_particles_additive.tech" : "instances_particles_combinative.tech";
      j["technique"] = jData.value<std::string>("technique", default_tech);
      j["casts_shadows"] = jData.value<bool>("casts_shadows", false);;
      j["category"] = jData.value<std::string>( "category", "transparent" );
      bool is_ok = material->create(j);

      // Assign a unique name and register it in the resources manager
      char name[64];
      sprintf(name, "particles_%p.material", &emitter);
      material->setNameAndType(name, getResourceTypeFor<TEmitter>());
      assert(is_ok);

      emitter.material = material;

      // Only if the material is custom
      auto ctes = new CCteBuffer<TCtesParticles>(CTE_BUFFER_SLOT_PARTICLES);
      ctes->create("ParticlesEmitter");
      ctes->psystem_frame_size = emitter.frameSize;
      ctes->psystem_nframes.x = 1.0f / emitter.frameSize.x;
      ctes->psystem_nframes.y = 1.0f / emitter.frameSize.y;
      ctes->updateGPU();
      material->ctes_material = ctes;
      emitter.ctes = ctes;
    }

    // sizes
    emitter.sizes = jData;

    emitter.colors = jData;
    emitter.sampleColorsOverTime();

    return true;
  }

  TRange CParser::loadRange(const json& jData, const std::string& attr, const TRange& defaultValue)
  {
    if (jData.count(attr) <= 0)
    {
      return defaultValue;
    }

    const json& jValue = jData[attr];
    if (jValue.is_array())
    {
      return TRange{ jValue.at(0), jValue.at(1) };
    }
    else
    {
      const float value = jData.value<float>(attr, 0.f);
      return TRange{ value, value };
    }
  }

  void TEmitter::sampleColorsOverTime() {
    colors.uniformSample(max_colors, ctes->psystem_colors_over_time);
    ctes->updateGPU();
  }

  void TEmitter::renderInMenu() {
    ImGui::DragFloat("Interval", &interval, 0.01f, 0.0f, 1.0f);
    ImGui::DragInt("Count", (int*) &count, 0.1f, 1, 64);
    ImGui::DragInt("Max Count", (int*)&maxCount, 0.1f, 1, 64);
    ImGui::DragFloat("Gravity", &gravityFactor, 0.01f, -10.0f, 10.0f);
    ImGui::DragFloat2("Duration", &duration.minValue, 0.01f, 0.0f, 10.0f);
    if (colors.renderInMenu("Colors"))
      sampleColorsOverTime();
  }

}
