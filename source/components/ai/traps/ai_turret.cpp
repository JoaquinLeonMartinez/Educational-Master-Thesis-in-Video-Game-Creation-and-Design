#include "mcv_platform.h"
#include "ai_turret.h"
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


DECL_OBJ_MANAGER("ai_turret", CAITurret);

void CAITurret::Init()
{
	AddState("IDLE",(statehandler)&CAITurret::IdleState);
	AddState("ALIGN", (statehandler)&CAITurret::AlignState);
	AddState("ATTACK", (statehandler)&CAITurret::AttackState);

	//pasive states
	AddState("IMPACT", (statehandler)&CAITurret::ImpactState);
	AddState("DEAD", (statehandler)&CAITurret::DeadState);


	ChangeState("IDLE");	
}

void CAITurret::IdleState(float dt) {

	if (!h_player.isValid()) {
		h_player = GameController.getPlayerHandle();
		return;
	}

	//nothing
	if (stunTimer > 0) { //STUN
		stunTimer -= dt;
	}
	else if(isView(length_cone)) {
		if (isFixed) {
			ChangeState("ATTACK");
		}
		else {
			ChangeState("ALIGN");
		}
	}
}

void CAITurret::AlignState(float dt) {

	//align to player
	TCompRender* render = get<TCompRender>();
	render->color = VEC4(1.0f, 1.0f, 1.0f, 1.0); //green

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);

	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) <= cone_vision) {
		ChangeState("ATTACK");
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

void CAITurret::AttackState(float dt) {

	TCompRender* render = get<TCompRender>();
	render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0); //blue

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//create bullet / cupcacke
	if (timerAttack > 0) {
		timerAttack -= dt;
	}
	else {
		timerAttack = attackFrequecy;

		//SHOOT BULLET
		c_trans->rotateTowards(player_position->getPosition(),90.f,dt);


		TEntityParseContext ctx;
		ctx.root_transform = *c_trans;
		parseScene("data/prefabs/bullets/turret_bullet.json", ctx);

		TMsgAssignBulletOwner msg;
		msg.h_owner = CHandle(this).getOwner();
		msg.source = c_trans->getPosition();
		msg.front = c_trans->getFront();
		ctx.entities_loaded[0].sendMsg(msg);

	}

	//if not align 
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) > cone_vision_forget && !isFixed) {
		ChangeState("ALIGN");
	}

	if (!isView(forgetEnemy)) {
		ChangeState("IDLE");
	}
}

template <typename T>
void CAITurret::Send_DamageMessage(CEntity* entity, float dmg) {
	//Send damage mesage
	T msg;
	// Who sent this bullet
	msg.h_sender = h_sender;
	// The bullet information ¿?¿?¿?
	msg.h_bullet = CHandle(this);
	msg.intensityDamage = dmg;
	entity->sendMsg(msg);

	dbg("ataque melee enviado al player \n");
}

void CAITurret::ImpactState(float dt) { //recibir damage real y no el que he puesto
	//if receive impact on back --> damage

	life = life - 10; //cambiar este por el damage que te envia el player
	stunTimer = stunTimeMax;
	
	if (life<=0) {
		ChangeState("DEAD");
	}
	else {
	ChangeState("IDLE");
	}
}

void CAITurret::DeadState(float dt) {
	//CHandle(this).getOwner().destroy();
	//CHandle(this).destroy();
}

bool CAITurret::isView(float distance) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distancia = VEC3::Distance(c_trans->getPosition(), player_position->getPosition());
	//Esta en cono
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if ( abs(angle) <= half_cone) {
		//Esta distancia 
		if (distancia <= distance) {
			return true;
		}
	}
	return false;
}

void CAITurret::load(const json& j, TEntityParseContext& ctx) {
	this->Init();
	half_cone = j.value("halfCone", half_cone);
	length_cone = j.value("lengthCone", length_cone);
	hearing_radius = j.value("hearingRadius", hearing_radius);
	life = j.value("life", life);
	isFixed = j.value("isFixed", isFixed);

	attackFrequecy = j.value("attackFrequecy", attackFrequecy);
	stunTimeMax = j.value("stunTimeMax", stunTimeMax);
	cone_vision = j.value("cone_vision", cone_vision);
	cone_vision_forget = j.value("cone_vision_forget", cone_vision_forget);
	forgetEnemyRange = j.value("forgetEnemyRange", forgetEnemyRange);
	damage = j.value("damage", damage);
	damageBullet = j.value("damageBullet", damageBullet);
	life = j.value("life", life);
	twistSpeed = j.value("twistSpeed", twistSpeed);
	
}

void CAITurret::debugInMenu() {
	ImGui::Text(state.c_str());
}

void CAITurret::renderDebug() {
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

void CAITurret::onDamageInfoMsg(const TMsgDamage& msg) {
	

	if (msg.senderType == PLAYER) {
		//hay que comprobar que el collider golpeado sea el del "sushi"
		//en ese caso se resta vida
		dbg(" Recibo el damage del player\n");

	}

}

void CAITurret::registerMsgs() {
	//DECL_MSG(CAITurret, TMsgOnContact, onCollision);
	//DECL_MSG(CAITurret, TMsgDamage, onDamageInfoMsg);
  DECL_MSG(CAITurret, TMsgAIPaused, onMsgAIPaused);
}
