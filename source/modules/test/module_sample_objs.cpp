#include "mcv_platform.h"
#include "render/render.h"
#include "module_sample_objs.h"
#include "engine.h"
#include "render/primitives.h"
#include "render/meshes/mesh_io.h"
#include "render/textures/texture.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "components/common/comp_name.h"

// -----------------------------------------------
void to_json(json& j, const CTransform& t) {
	// Need to simplify this.
	VEC3 v = t.getPosition();
	char txt[64];
	sprintf(txt, "%f %f %f", v.x, v.y, v.z);
	j["pos"] = txt;

	// And save rotation
}

void from_json(const json& j, CTransform& t) {
	t.load(j);
}


CModuleSampleObjs::CModuleSampleObjs(const std::string& name)
	: IModule(name)
{}

bool CModuleSampleObjs::start()
{
	return true;
}

void CModuleSampleObjs::stop()
{
}

void CModuleSampleObjs::update(float dt)
{
}

void CModuleSampleObjs::renderInMenu() {

}

void CModuleSampleObjs::renderDebug() {

}
