#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "datatypes.h"
#include "module_blackBoard.h"




class CModuleBlackBoard : public IModule
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
	void runScript(ScriptType scriptType, const std::string& params = "", float delay = 0.f);
	void execAction(const std::string& action);
	void execActionDelayed(const std::string & script, float delay);

};

void saludar();
void immortal(bool active);
CModuleGameController* getGameController();
