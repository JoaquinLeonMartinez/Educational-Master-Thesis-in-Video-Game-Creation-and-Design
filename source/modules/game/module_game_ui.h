#pragma once

#include "modules/module.h"
#include "entity/entity.h"



class CModuleGameUI : public IModule
{

public:

  CModuleGameUI(const std::string& aname) : IModule(aname) { }
  bool start() override;
  void stop() override;
  void update(float delta) override;
  void renderDebug() override;
  void renderInMenu() override;

private:
	void onOptionCarrito();
	void onOptionDash();
	void onOptionJump();
	void onOptionMop();

  float actualLifeRatioBar = -1.0f;
  float actualMadnessRatioBar = -1.0f;
};

