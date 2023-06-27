#include "mcv_platform.h"
#include "comp_bullet.h"
#include "components/common/comp_transform.h"
#include "entity/entity_parser.h"
#include "engine.h"
#include "input/input.h"
#include "input/module_input.h"

DECL_OBJ_MANAGER("bullet_controller", TCompBulletController);

void TCompBulletController::debugInMenu() {
  ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 10.f);
} 

void TCompBulletController::load(const json& j, TEntityParseContext& ctx) {
  speed = j.value("speed", speed);
}

void TCompBulletController::registerMsgs() {
  DECL_MSG(TCompBulletController, TMsgAssignBulletOwner, onBulletInfoMsg);
}

void TCompBulletController::onBulletInfoMsg(const TMsgAssignBulletOwner& msg) {
  h_sender = msg.h_owner;

  TCompTransform* c_trans = get<TCompTransform>();
  c_trans->setPosition(msg.source);
  float yaw = vectorToYaw(msg.front);
  c_trans->setAngles(yaw, 0.f, 0.f);
}

void TCompBulletController::update(float delta) {

  TCompTransform* c_trans = get<TCompTransform>();
  if (!c_trans)
    return;
  VEC3 new_pos = c_trans->getPosition() + c_trans->getFront() * speed * delta;
  c_trans->setPosition(new_pos);

  // ---------------------
  // Check for collisions 
  auto om_tmx = getObjectManager<TCompTransform>();

  // This will store the handle close enough to me
  CHandle h_near;
  om_tmx->forEach([this, &h_near, c_trans](TCompTransform* c) {

	  // Discard myself
	  if (c_trans == c)
		  return;

	  float distance_to_c = VEC3::Distance(c->getPosition(), c_trans->getPosition());
	  if (distance_to_c < this->collision_radius) {
		  CHandle h_candidate = CHandle(c).getOwner();

		  // Discard my sender (the teapot)
		  if (h_candidate != h_sender)
			  h_near = h_candidate;
	  }
  });

  // 
  if (h_near.isValid()) {
	  CEntity *e_near = h_near;

	  dbg("Entity %s has been hit\n", e_near->getName());

	  // Notify the entity that he has been hit
    TMsgDamageToEnemy msg;
	  msg.h_sender = h_sender;      // Who send this bullet
	  msg.h_bullet = CHandle(this); // The bullet information
	  msg.intensityDamage = 20;
	  e_near->sendMsg(msg);

	  // Queue my owner entity as 'destroyed'
	  CHandle(this).getOwner().destroy();
  }
}


