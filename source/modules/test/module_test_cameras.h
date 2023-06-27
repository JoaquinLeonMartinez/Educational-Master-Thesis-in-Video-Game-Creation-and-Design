#pragma once
#include "modules/module.h"

class CCurve;

class CModuleTestCameras : public IModule
{
public:
  CModuleTestCameras(const std::string& name);
  bool start() override;
  void stop() override;
  void update(float dt) override;
  void renderDebug() override;

private:
  const CCurve* _curve = nullptr;
  int targetCamera = 1;
};