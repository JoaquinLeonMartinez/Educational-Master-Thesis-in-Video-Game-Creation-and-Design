#pragma once

#include "mcv_platform.h"

namespace UI
{
  class CWidget;

  class CEffect
  {
  public:
    virtual void start() {}
    virtual void stop() {}
    virtual void update(float dt) {}
	virtual void onDeactivate() {};
	virtual void stopUiFx() {};
	virtual void changeSpeedUV(float x, float y) {};
	virtual void changeDuration(float duration) {};

	std::string getName() {
		return name;
	}
  protected:
    CWidget* _owner = nullptr;
	std::string name = "";
    friend class CParser;
  };
}
