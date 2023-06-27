#include "mcv_platform.h"
#include "comp_id.h"

DECL_OBJ_MANAGER("id", TCompID);


std::unordered_map< int, CHandle > TCompID::all_ids;

void TCompID::debugInMenu() {
	ImGui::InputInt("id", &_id);
}

void TCompID::setId(int id) {
	//TODO: COMPROBAR SI EXISTE
	if (!all_ids[id].isValid()) {
		_id = id;
		all_ids[_id] = CHandle(this);
	}
	else {
		assert(!all_ids[id].isValid());//el id ya existe
	}

}
 
void TCompID::load(const json& j, TEntityParseContext& ctx) {
	_id = j.get<int>();
	setId(_id);
}



CHandle getEntityById(const int& id) {

	auto it = TCompID::all_ids.find(id);
	if (it == TCompID::all_ids.end())
		return CHandle();

	CHandle h_id = it->second;
	return h_id.getOwner();
}
