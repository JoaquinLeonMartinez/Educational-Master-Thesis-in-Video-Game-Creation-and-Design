#include "mcv_platform.h"
#include "comp_checkpoint_register.h"
#include "components/common/comp_transform.h"
#include "engine.h"
#include "components/common/comp_tags.h"

using namespace physx;

DECL_OBJ_MANAGER("comp_checkpoint_register", TCompCheckpointRegister);

/*

Purpose:

This component marks the entity to be saved and loaded once a checkpoint is triggered.

Behavior:

Once the entity is loaded, it's prefab's address and EntityType are stored
Once the entity is created, it assigns itself the 'checkpoint_registered'

*/

void TCompCheckpointRegister::debugInMenu() {
    ImGui::Text(prefab.c_str());
    ImGui::Text(entityTypeString.c_str());
}

void TCompCheckpointRegister::load(const json& j, TEntityParseContext& ctx) {
    prefab = j.value("prefab", prefab);
    entityTypeString = j.value("entityTypeString", entityTypeString);
    
    if (strcmp("PLAYER", entityTypeString.c_str()) == 0) {
        entityType = EntityType::PLAYER;
    }
    else if (strcmp("CUPCAKE", entityTypeString.c_str()) == 0) {
        entityType = EntityType::CUPCAKE;
    }
    else if (strcmp("SUSHI", entityTypeString.c_str()) == 0) {
        entityType = EntityType::SUSHI;
    }
    else if (strcmp("RANGED_SUSHI", entityTypeString.c_str()) == 0) {
        entityType = EntityType::RANGED_SUSHI;
    }
    else if (strcmp("GOLEM", entityTypeString.c_str()) == 0) {
        entityType = EntityType::GOLEM;
    }
    else if (strcmp("CUPCAKE_SPAWNER", entityTypeString.c_str()) == 0) {
        entityType = EntityType::CUPCAKE_SPAWNER;
    }
    else if (strcmp("EXPLOSIVE_OBJECT", entityTypeString.c_str()) == 0) {
        entityType = EntityType::EXPLOSIVE_OBJECT;
    }
    else if (strcmp("DUMPLING_TURRET", entityTypeString.c_str()) == 0) {
        entityType = EntityType::DUMPLING_TURRET;
    }
	else if (strcmp("SCART", entityTypeString.c_str()) == 0) {
		entityType = EntityType::SCART;
	}
	else if (strcmp("GRENADE_GOLEM", entityTypeString.c_str()) == 0) {
		entityType = EntityType::GRENADE_GOLEM;
	}
	else if (strcmp("BUTCHER_TRAP", entityTypeString.c_str()) == 0) {
		entityType = EntityType::BUTCHER_TRAP;
	}
	else if (strcmp("EXPLOSIVE_CUPCAKE", entityTypeString.c_str()) == 0) {
		entityType = EntityType::EXPLOSIVE_CUPCAKE;
	}
	else if (strcmp("CUPCAKE_SPAWNER_SPECIAL", entityTypeString.c_str()) == 0) {
		entityType = EntityType::CUPCAKE_SPAWNER_SPECIAL;
	}

    assert(prefab != "UNDEFINED");
    assert(entityTypeString != "UNDEFINED");
    assert(entityType != UNDEFINED);
}

void TCompCheckpointRegister::registerMsgs() {
  DECL_MSG(TCompCheckpointRegister, TMsgEntityCreated, onCreation);
}

/*

Once the entity is created, assign 'checkpoint_registered' tag to itself

*/
void TCompCheckpointRegister::onCreation(const TMsgEntityCreated& msgC) {
    TCompTags* c_tag = get<TCompTags>();
    if (!c_tag)
        fatal("Component 'tags' is necessary for an entity to be registered into checkpoints.");
    c_tag->addTag(getID("checkpoint_registered"));
}

void TCompCheckpointRegister::update(float dt) {
}

std::string TCompCheckpointRegister::getPrefab() {
    return prefab;
}

EntityType TCompCheckpointRegister::getEntityType() {
    return entityType;
}