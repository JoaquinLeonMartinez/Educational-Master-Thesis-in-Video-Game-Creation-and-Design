#pragma once

#include "modules/module.h"

class CModuleMainMenu : public IModule
{
public:
  CModuleMainMenu(const std::string& name);
  bool start() override;
  void stop() override;
  void update(float dt) override;

private:
  void onOptionStart();
  void onOptionContinue();
  void onOptionExit();
};