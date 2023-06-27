#include "mcv_platform.h"
#include "comp_trail.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"

DECL_OBJ_MANAGER("trail", TCompTrail);

void TCompTrail::debugInMenu() {

}

void TCompTrail::load(const json& j, TEntityParseContext& ctx) {

}

void TCompTrail::registerMsgs() {
  DECL_MSG(TCompTrail, TMsgEntityCreated, onCreation);
}

void TCompTrail::onCreation(const TMsgEntityCreated& msgC) {
  /*auto rmesh = Resources.get("trail_mesh.mesh")->as<CMesh>();
  trail_mesh = (CMesh*)rmesh;*/
}

void TCompTrail::renderDebug() {

}

void TCompTrail::update(float delta) {
  //updateVertsFromCPU(data, total_instances * bytes_per_instance);
  //TCompRender* c_render = get<TCompRender>();
  //c_render->parts[0].mesh->updateVertsFromCPU(vtxs.data(), (size_t)(sizeof(SimpleVertex) * vtxs.size()));
}


