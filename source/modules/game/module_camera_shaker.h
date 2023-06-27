#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "utils/PerlinNoise.hpp"

//https://github.com/Reputeless/PerlinNoise
//class siv::PerlinNoise;

class CModuleCameraShaker : public IModule
{
  float trauma = 0.f;
  float traumaDecreasingSpeed = 0.1f;
  float minTrauma = 0.f;
  float shake = 0.f;

  float maxYaw = 2.f;
  float maxPitch = 2.f;
  float maxRoll = 3.f; //15 degrees

  float timeToReturn = 0.f;

  std::vector<siv::PerlinNoise> perlin_noise;

public:

  float yawOffset = 0.f;
  float pitchOffset = 0.f;
  float rollOffset = 0.f;

  CModuleCameraShaker(const std::string& aname) : IModule(aname) { }
  bool start() override;
  void stop() override;
  void update(float delta) override;
  void renderDebug() override;
  void renderInMenu() override;

  void addTrauma(float amount);
  void setMinTrauma(float amount);
};

