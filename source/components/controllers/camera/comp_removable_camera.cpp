#include "mcv_platform.h"
#include "comp_removable_camera.h"
#include "components/common/comp_transform.h"
#include "engine.h"
#include "input/input.h"
#include "modules/module_camera_mixer.h"
#include "components/common/comp_render.h"

DECL_OBJ_MANAGER("removable_camera", TCompRemovableCamera);

void TCompRemovableCamera::debugInMenu()
{

}

void TCompRemovableCamera::load(const json& j, TEntityParseContext& ctx) {

}

void TCompRemovableCamera::update(float scaled_dt)
{
  CHandle me(this);
  CHandle c = me.getOwner();
  float weight = Engine.getCameraMixer().getCameraWeight(c);
  if (weight == -1.f) { //CAMERA NOT FOUND
    CHandle(this).getOwner().destroy();
    CHandle(this).destroy();
  }
}

void TCompRemovableCamera::renderDebug()
{

}
