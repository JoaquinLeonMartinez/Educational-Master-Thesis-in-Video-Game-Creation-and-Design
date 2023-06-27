#pragma once

#include "modules/module.h"

class CMeshInstanced;

class CModuleFluidDecalGenerator : public IModule {

  float unitRandom();

  float randomFloat(float vmin, float vmax);

  // -------------------------------------------------------------------
  struct TInstanceBlood {
    MAT44 world;
    VEC4  color;
    float time_stamp;
    float time_transition = 25.f;
    VEC2  other = VEC2::Zero;
  };
  CMeshInstanced* blood_instances_mesh = nullptr;
  std::vector< TInstanceBlood > blood_instances;

  float removalPeriod = 60.f;
  float actualTime = 0.f;

  // -------------------------------------------------------------------
public:
  CModuleFluidDecalGenerator(const std::string& name);
  bool start() override;
  void renderDebug() override;
  void renderInMenu() override;
  void update(float dt) override;
  void generateFluid(int impactForce, VEC3 position = VEC3::Zero, VEC3 normal = VEC3::Zero);
  void generateSingleFluidUncapped(float scale, VEC3 exactPosition = VEC3::Zero, VEC3 normal = VEC3::Zero);
};