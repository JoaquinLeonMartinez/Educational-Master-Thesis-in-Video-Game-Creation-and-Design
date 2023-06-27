#pragma once

#include "modules/module.h"
#include "entity/entity.h"



class CModuleWinGame : public IModule
{

public:

	CModuleWinGame(const std::string& aname) : IModule(aname) { }
	bool start() override;
	void stop() override;
	void update(float delta) override;
	void renderDebug() override;
	void renderInMenu() override;

private:
	float timeOffModuleGameWin = 1000.0f;
};

