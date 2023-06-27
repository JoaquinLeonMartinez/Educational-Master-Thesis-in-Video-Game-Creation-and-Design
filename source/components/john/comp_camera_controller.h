#pragma once

#include "components/common/comp_base.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_camera.h"
#include "entity/entity.h"
#include "render/render.h"



class TCompCameraController : public TCompBase {

 // CCteBuffer<TCtesCamera> ctes_camera(CTE_BUFFER_SLOT_CAMERAS);
  float rotation_speed = 0.0f;
  float speed = 2.0f;
  TCompTransform* c_trans;
  TCompCamera* c_camera;
  

  DECL_SIBLING_ACCESS();

public:
  void update(float dt);
  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
 
private:
	bool camera_type = true;

};