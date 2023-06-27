#pragma once
#include "modules/module.h"

class CModuleLogic : public IModule
{
public:
	CModuleLogic(const std::string& name) : IModule(name) { }
	bool start() override;
	void stop() override;
	void update(float dt) override;
	void renderInMenu() override;
	CModuleLogic* getPointer() { return this; }
	void deleteEnemies();
	CHandle spawnBot(VEC3 &position, const VEC3 & lookat,std::string &type);
	void move(const std::string & name, const VEC3 & pos, const VEC3 & lookat);
private:
  float _timer = 0.f;
};

CModuleLogic* getLogic();
