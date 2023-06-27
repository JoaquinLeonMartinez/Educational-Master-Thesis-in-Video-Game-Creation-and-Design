#include "mcv_platform.h"
#include "ai_golem.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/john/comp_bullet.h"
#include "components/common/comp_tags.h"
#include "time.h"
#include "stdlib.h"
#include "modules/module_physics.h"

using namespace physx;


DECL_OBJ_MANAGER("ai_golem", CAIGolem);

void CAIGolem::Init()
{
	AddState("IDLE", (statehandler)&CAIGolem::IdleState);
	//AddState("IDLECOMBAT", (statehandler)&CAIGolem::IdleCombatState);
	AddState("ALIGN", (statehandler)&CAIGolem::AlignState);
	AddState("THROW", (statehandler)&CAIGolem::ThrowState);
	AddState("MELEE", (statehandler)&CAIGolem::MeleeState);
	AddState("BATTERYEFECT", (statehandler)&CAIGolem::BatteryEfectState);

	//pasive states
	AddState("IMPACT", (statehandler)&CAIGolem::ImpactState);
	AddState("DEAD", (statehandler)&CAIGolem::DeadState);


	ChangeState("IDLE");
}


void CAIGolem::IdleState(float dt) {

	if (!h_player.isValid()) {
		h_player = getEntityByName("Player");
		return;
	}

	//nothing

	if (isView(length_cone)) { //comprobar el cono de vision
		ChangeState("ALIGN");
	}

}

void CAIGolem::AlignState(float dt) {

	////align to player
	//TCompRender* render = get<TCompRender>();
	//render->color = VEC4(1.0f, 1.0f, 1.0f, 1.0); //green

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);

	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) <= cone_vision) {

		if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) < meleeRange) {
			ChangeState("MELEE");
		}
		else {
			ChangeState("THROW");
		}

	}
	else {
		if (c_trans->isInLeft(player_position->getPosition())) {
			c_trans->setAngles(yaw + dt * twistSpeed, pith);
		}
		else {
			c_trans->setAngles(yaw - dt * twistSpeed, pith);
		}
	}

	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) > forgetEnemy) {
		ChangeState("IDLE");
	}

}

void CAIGolem::ThrowState(float dt) {

	//TCompRender* render = get<TCompRender>();
	//render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0); //blue

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//create bullet / cupcacke
	if (timerGrenade > 0) {
		timerGrenade -= dt;
	}
	else {
		dbg("lanza granada");
		timerGrenade = throwFrequecy;
		TEntityParseContext ctx;
		ctx.root_transform = *c_trans;
		parseScene("data/prefabs/bullets/grenade_golem.json", ctx); //cambiar granada por algo que me invente que no explote

		TMsgAssignBulletOwner msg;
		msg.h_owner = CHandle(this).getOwner();
		msg.source = c_trans->getPosition();
		msg.front = c_trans->getFront();
		ctx.entities_loaded[0].sendMsg(msg);
	}


	//throw it to player position


	//if not align 
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) > cone_vision_forget) {
		ChangeState("ALIGN");
	}
	//if forget
	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) > forgetEnemy) {
		ChangeState("IDLE");
	}

	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) < meleeRange) {
		ChangeState("MELEE");
	}
}


template <typename T>
void CAIGolem::Send_DamageMessage(CEntity* entity, float dmg) {
	//Send damage mesage
	T msg;
	// Who sent this bullet
	msg.h_sender = h_sender;
	// The bullet information �?�?�?
	msg.h_bullet = CHandle(this).getOwner();
	msg.intensityDamage = dmg;
	entity->sendMsg(msg);

	dbg("ataque melee enviado al player \n");
}

void CAIGolem::MeleeState(float dt) {

	//TCompRender* render = get<TCompRender>();
	//render->color = VEC4(1.0f, 0.0f, 0.0f, 1.0); //red

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//send damage msg to player if is in the meleeRange

	if (timerMelee > 0) {//delay
		timerMelee -= dt;
	}
	else {
		timerMelee = meleeFrequency;

		if (isView(meleeRange)) {
			//crear esfera de colision
			PxSphereGeometry geometry(meleeRange);
			Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeRange);
			PxVec3 pos = VEC3_TO_PXVEC3(damageOrigin);
			PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());
			PxTransform shapePose = PxTransform(pos, ori);    // [in] initial shape pose (at distance=0)
			PxOverlapBuffer hit;
			//Evaluate collisions
			bool res = EnginePhysics.gScene->overlap(geometry, shapePose, hit, PxQueryFilterData(PxQueryFlag::eANY_HIT | PxQueryFlag::eDYNAMIC));
			if (hit.hasAnyHits()) {
				//Analyze entities hit
				for (int i = 0; i < hit.getNbAnyHits(); i++) {
					CHandle h_comp_physics;
					h_comp_physics.fromVoidPtr(hit.getAnyHit(i).actor->userData);
					CEntity* entityContact = h_comp_physics.getOwner();
					if (entityContact) {
						TCompTags* c_tag = entityContact->get<TCompTags>();
						if (c_tag) {
							//If the entity's tag is "player"
							std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
							if (strcmp("player", tag.c_str()) == 0) {
								Send_DamageMessage<TMsgDamageToPlayer>(entityContact, damage);
							}
						}
					}
				}
			}
		}
	}

	//if not align 
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) > cone_vision_forget) {
		ChangeState("ALIGN");
	}
	//if forget
	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) > forgetEnemy) {
		ChangeState("IDLE");
	}

	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) > meleeRange + 1.0f) {
		ChangeState("THROW");
	}
}




void CAIGolem::ImpactState(float dt) {
	//if receive impact on back --> damage
	//dbg("recibe el impacto\n");
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);

	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) > half_cone) {
		dbg("recibe el impacto en la espalda\n");
		//si esta fuera del cono de vision es que esta en la esalda
		life = life - 50; //cambiar este por el damage que te envia el player
	}
	

	if (life <= 0) {
		ChangeState("DEAD");
	}
	else {
		ChangeState("IDLE");
	}
}

void CAIGolem::DeadState(float dt) {
	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();
}

bool CAIGolem::isView(float distance) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distancia = VEC3::Distance(c_trans->getPosition(), player_position->getPosition());
	//Esta en cono
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) <= half_cone) {
		//Esta distancia 
		if (distancia <= distance) {
			return true;
		}
	}
	return false;
}

void CAIGolem::load(const json& j, TEntityParseContext& ctx) {
	this->Init();
	half_cone = j.value("halfCone", half_cone);
	length_cone = j.value("lengthCone", length_cone);
	hearing_radius = j.value("hearingRadius", hearing_radius);
	life = j.value("life", life);
	damageArea = j.value("damageArea", damageArea);

}

void CAIGolem::debugInMenu() {
	ImGui::Text(state.c_str());
}

void CAIGolem::renderDebug() {
	TCompTransform* c_trans = get<TCompTransform>();
	TCompRender* c_render = get<TCompRender>();
	VEC3 front = c_trans->getFront();
	VEC3 pos = c_trans->getPosition();
	float angle = deg2rad(half_cone);

	//Create a rotation matrix with the angle
	MAT44 aux_cone_1 = MAT44::CreateRotationY(angle);
	MAT44 aux_cone_2 = MAT44::CreateRotationY(-angle);

	//Create two vectors to store the result
	VEC3 half_cone_1, half_cone_2;

	//We rotate the vector "front" with the matrix "aux_cone_X" into "half_cone_X"
	VEC3::Transform(front, aux_cone_1, half_cone_1);
	VEC3::Transform(front, aux_cone_2, half_cone_2);
	half_cone_1.Normalize();
	half_cone_2.Normalize();

	drawCircle(pos, hearing_radius, c_render->color);
	drawLine(pos, pos + half_cone_1 * length_cone, c_render->color);
	drawLine(pos, pos + half_cone_2 * length_cone, c_render->color);
	drawLine(pos + half_cone_1 * length_cone, pos + half_cone_2 * length_cone, c_render->color);
}

void CAIGolem::onCollision(const TMsgOnContact& msg) {
	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();

	TCompTags* c_tag = source_of_impact->get<TCompTags>();
	if (c_tag) {
		std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
		if (strcmp("floor", tag.c_str()) == 0) {
			return;
		}
		else if (strcmp("enemy", tag.c_str()) == 0) {
			return;
		}
		else if (strcmp("player", tag.c_str()) == 0) {
			return;
		}
		else if(strcmp("grenade", tag.c_str()) == 0){
			ChangeState("IMPACT");
		}
		else if (strcmp("bullet", tag.c_str()) == 0) {
			ChangeState("IMPACT");
		}
	}
}

void CAIGolem::onGravity(const TMsgGravity& msg) {
	//	Paralyze golem during a timer

	h_sender = msg.h_sender;
	h_bullet = msg.h_bullet;
	//obtener posicion

	CEntity* e_bullet = (CEntity *)h_bullet;
	TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
	//battery_position = bullet_trans->getPosition();
	battery_time = msg.time_effect;


	ChangeState("BATTERYEFECT");

}

void CAIGolem::BatteryEfectState(float dt) {

	if (battery_time > 0) {
		battery_time -= dt;
		//NOTHING

		dbg("battery time = %f \n", battery_time);
	}
	else {
		dbg("entra en align");
		ChangeState("ALIGN");
	}

}

void CAIGolem::registerMsgs() {
	DECL_MSG(CAIGolem, TMsgOnContact, onCollision);
	DECL_MSG(CAIGolem, TMsgGravity, onGravity);
}
