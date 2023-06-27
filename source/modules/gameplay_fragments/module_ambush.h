#pragma once

#include "modules/gameplay_fragments/module_gameplay_fragment.h"


class CModuleAmbush : public CModuleGameplayFragment
{
  int sushisKilled = 0;
  int termostatosActives = 0;
  void firstEvent();
  void secondEvent();

public:

  CModuleAmbush(const int& difficulty) : CModuleGameplayFragment(difficulty) { }
  bool start() override;
  void stop() override;
  void renderInMenu() override;

  void enemyKilled(EntityType e);
  int getThermostatsActivates();
   
};

