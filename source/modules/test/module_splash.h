#pragma once
#include "modules/module.h"

class CModuleSplash : public IModule
{
public:
  CModuleSplash(const std::string& name);
  bool start() override;
  void stop() override;
  void update(float dt) override;

private:
  float _timer = 0.f;
};