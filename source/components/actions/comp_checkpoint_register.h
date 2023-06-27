#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"

class TCompCheckpointRegister : public TCompBase {

	
	DECL_SIBLING_ACCESS();

public:
	void update(float dt);
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	static void registerMsgs();

  std::string getPrefab();
  EntityType getEntityType();

private:
    std::string entityTypeString = "UNDEFINED";
    EntityType entityType = UNDEFINED;
    std::string prefab = "undefined";

  void onCreation(const TMsgEntityCreated& msg);
};