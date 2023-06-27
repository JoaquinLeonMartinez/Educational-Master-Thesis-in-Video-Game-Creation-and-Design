#pragma once

#include "modules/module.h"

class CModuleGameplay : public IModule
{
public:
  CModuleGameplay(const std::string& name);

  bool start() override;
  void stop() override;
  void update(float dt) override;
};