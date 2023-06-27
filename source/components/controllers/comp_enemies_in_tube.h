#pragma once

#include "components/common/comp_base.h"
#include "components/common/comp_name.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"


class TCompEnemiesInTube : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void load(const json& j, TEntityParseContext& ctx);//meter en jason la velocidad de lanzamiento
  void debugInMenu();
  void update(float dt);
  static void registerMsgs();
  bool activateTrap = false;

  void setActivateTrap(bool value);

  CHandle h1;
  CHandle h2;
  CHandle h3;
  CHandle h4;

  bool activateTrap1 = false;
  bool activateTrap2 = false;
  bool activateTrap3 = false;
  bool activateTrap4 = false;
  
private:
	std::vector<VEC3> enemiesPosition;
	float birthTime = 5.f;
	float birthTimeReset = 5.f;
	float limitTime = 0.f;
	int positionBirth = 0;
	bool firstTime = false;

	void onFinishEnemiesInTube(const TMSgWallDestroyed & msg);
	
};

