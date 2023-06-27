#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "datatypes.h"
#include "module_game_controller.h"




class CModuleScripting : public IModule
{
	struct DelayedAction { 
		std::string action;
		float time;
	};
	std::vector<DelayedAction> delayedActions;

public:

	CModuleScripting(const std::string& aname) : IModule(aname) { }
	CModuleScripting() : IModule("KK") {};
	
    bool start() override;
    void update(float delta) override;
    void stop() override;

	void loadScriptsInFolder(const char * path);
	void doBingings();
	void BindGameController();
	void BindCharacterController();
	void BindGeometry();
	void BindGlobalFunctions();
	void BindHandle();
	void BindTransform();
	void BindSkeleton();
	void BindConverters();
	void BindEnemiesInTube();
	void BindName();
	void BindCamera();
	void BindGolem();
	void BindEnemySpawner();
	void BindSelfDestroy();
	void BindPointLights();
	void BindFlickering();
	void BindBalanceo();
	void BindDirectionalLights();

	void runScript(std::string nameScript, const std::string& params = "", float delay = 0.f);
	bool execAction(const std::string& action);
	bool execActionDelayed(const std::string & script, float delay);
	bool execEvent(Events event, const std::string& params = "", float delay = 0.f);
};

void saludar();
void immortal(bool active);
void execDelayedAction(const std::string &action, float delay);
CModuleGameController* getGameController();
