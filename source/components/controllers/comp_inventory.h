#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "geometry/curve.h"

class TCompInventory : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void renderDebug();

  
  void setBattery(bool value) { 
	  battery = value;
  }
  void setTeleport(bool value) { 
	  teleport = value;
  }
  void setChilli(bool value) { 
	  chilli = value;
  }
  void setCoffe(bool value) { 
	  coffe = value;
  }

  bool getBattery() {
	  return battery;
  }
  bool getTeleport() {
	  return teleport;
  }
  bool getChilli() {
	  return chilli;
  }
  bool getCoffe() {
	  return coffe;
  }

  
  void update(float delta);

private:
	bool battery = false;
	bool teleport = true;
	bool chilli = false;
	bool coffe = false;
};

