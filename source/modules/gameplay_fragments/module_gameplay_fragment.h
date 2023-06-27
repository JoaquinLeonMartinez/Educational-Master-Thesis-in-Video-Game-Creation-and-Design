#pragma once
#include "entity/entity.h"
#include "engine.h"

class CModuleGameplayFragment
{
  int _difficulty;

public:


  CModuleGameplayFragment(const int& difficulty)
  {
    _difficulty = difficulty;
  }
  virtual bool start() { return true; }
  virtual void stop() { }
  virtual void renderInMenu() {}


  //EVENTS OF GAMEPLAY FOR EXTRA SCRIPTING
  virtual void enemyKilled(EntityType e) {}
  virtual int getThermostatsActivates() { return 0; }
  virtual void secondEvent() {};
};
