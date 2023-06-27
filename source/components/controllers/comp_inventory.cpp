#include "mcv_platform.h"
#include "comp_inventory.h"
#include "components/common/comp_transform.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("comp_inventory", TCompInventory);

void TCompInventory::debugInMenu() {
	ImGui::Checkbox("UnLockable Battery", &battery);
	ImGui::Checkbox("UnLockable Teleport", &teleport);
	ImGui::Checkbox("UnLockable Chilli", &chilli);
	ImGui::Checkbox("UnLockable Coffe", &coffe);
}


void TCompInventory::load(const json& j, TEntityParseContext& ctx) {
	coffe = j.value("coffe", coffe);
	battery = j.value("battery", battery);
	chilli = j.value("chilli", chilli);
	teleport = j.value("teleport", teleport);
}

void TCompInventory::update(float delta) {
  
}



void TCompInventory::renderDebug() {
 
}
