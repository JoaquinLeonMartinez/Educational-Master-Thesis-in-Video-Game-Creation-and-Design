#pragma once

#include "modules/module.h"
#include "render/meshes/mesh.h"
#include "render/render.h"
#include "geometry/transform.h"
#include "utils/json_resource.h"

class CModuleSampleObjs : public IModule
{

public:

public:
	CModuleSampleObjs(const std::string& name);

	bool start() override;
	void stop() override;
	void renderDebug() override;
	void renderInMenu() override;
	void update(float dt) override;
};