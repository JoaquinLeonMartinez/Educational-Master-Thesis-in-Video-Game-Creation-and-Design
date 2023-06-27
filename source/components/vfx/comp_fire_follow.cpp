#include "mcv_platform.h"

#include "comp_fire_follow.h"
#include "components/common/physics/comp_collider.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/powers/comp_fire.h"
#include "components/common/comp_render.h"

DECL_OBJ_MANAGER("fire_follow", TCompFireFollow);

void TCompFireFollow::debugInMenu() {

} 

void TCompFireFollow::load(const json& j, TEntityParseContext& ctx) {
}

void TCompFireFollow::update(float delta) {
  CEntity* en = getEntityByName("Player");
  CEntity* ex = getEntityByName("Anti_extintor");
  if (!en && !ex)
      return;
  TCompTransform* c_trans_ex = ex->get<TCompTransform>();
  TCompTransform* c_trans_pl = en->get<TCompTransform>();
  //emision position

  TCompBuffers* c_buff = get<TCompBuffers>();
  if (c_buff) {
    auto buf = c_buff->getCteByName("TCtesParticles");
    CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
    data->emitter_center = c_trans_ex->getPosition();

    TCompCharacterController* p_contr = en->get<TCompCharacterController>();
    if (p_contr->aiming) { //emision direction
      CEntity* cam = getEntityByName("PlayerCamera");
      TCompTransform* cam_trans = cam->get<TCompTransform>();
      data->emitter_dir = cam_trans->getFront();
    }else {
      data->emitter_dir = c_trans_pl->getFront();
    }

    //bool emision
    TCompFireController* c_fire = en->get<TCompFireController>();
    TCompRender* c_render = get<TCompRender>();
    if (c_fire->isEnabled()) {
      data->emitter_num_particles_per_spawn = 20;
    }
    else {
      data->emitter_num_particles_per_spawn = 0;
    }
    data->updateGPU();
	}
}




