#include "mcv_platform.h"
#include "ai_patrol_soldier.h"
#include "entity/entity.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/common/comp_life.h"
#include "components/john/comp_bullet.h"

#include "time.h"
#include "stdlib.h"
DECL_OBJ_MANAGER("ia_patrol_soldier", CAIPatrol_Soldier);

void CAIPatrol_Soldier::Init()
{
	AddState("SEEKWPT",(statehandler)&CAIPatrol_Soldier::SeekwptState);
	AddState("NEXTWPT",(statehandler)&CAIPatrol_Soldier::NextwptState);
	AddState("CHASE",(statehandler)&CAIPatrol_Soldier::ChaseState);
	AddState("IDLWAR", (statehandler)&CAIPatrol_Soldier::IdlWarState);
	AddState("ORBITLEFT", (statehandler)&CAIPatrol_Soldier::OrbitLeftState);
	AddState("ORBITRIGHT", (statehandler)&CAIPatrol_Soldier::OrbitRightState);
	AddState("IMPACT", (statehandler)&CAIPatrol_Soldier::ImpactState);
	AddState("DEAD", (statehandler)&CAIPatrol_Soldier::DeadState);

	ChangeState("SEEKWPT");

	
	positions.push_back(VEC3(5.0f, 0.0f, 5.0f));
	positions.push_back(VEC3(0.0f, 0.0f, 0.0f));
	positions.push_back(VEC3(5.0f, 0.0f, -5.0f));
	
	nextPoint = positions[i];
		
}

void CAIPatrol_Soldier::SeekwptState(float dt) {
	/*if (!h_player.isValid()) {
		h_player = getEntityByName("Player");
		return;
	}

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	currentPosition = c_trans->getPosition();
	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);
	
	float yaw_to_wpt = c_trans->getDeltaYawToAimTo(nextPoint);

	if (abs(rad2deg(yaw_to_wpt)) < 1.0f ) {
		ChangeState("NEXTWPT");
	} else {
		if (c_trans->isInLeft(nextPoint)) {
			c_trans->setAngles(yaw + dt, pith);
		}
		else {
			c_trans->setAngles(yaw - dt, pith);
		}
	}
	bool flag = isView();
	if (flag) {
		ChangeState("CHASE");
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(0.0f, 1.0f, 0.0f, 1.0);
	}
	*/
}

void CAIPatrol_Soldier::NextwptState(float dt) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	VEC3 pos = c_trans->getPosition();
	VEC3 res;
	c_trans->getFront().Normalize(res);
	pos = pos + res * dt * speed;
	c_trans->setPosition(pos);
	float distancia = VEC3::Distance(nextPoint, c_trans->getPosition());

	
	if (distancia < 1.0f ) {
		i = (i + 1) % positions.size();
		nextPoint = positions[i];

		ChangeState("SEEKWPT");
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0);
	}
	bool flag = isView();
	if (flag) {
		ChangeState("CHASE");
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(0.0f, 1.0f, 0.0f, 1.0);
	}
	
}

void CAIPatrol_Soldier::ChaseState(float dt) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	VEC3 res;

	VEC3 pos = c_trans->getPosition();
	c_trans->getFront().Normalize(res);
	pos = pos + res * dt * speed;
	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);
	c_trans->setPosition(pos);
	if (c_trans->isInLeft(player_position->getPosition())) {
		c_trans->setAngles(yaw + dt, pith);
	}
	else {
		c_trans->setAngles(yaw - dt, pith);
	}

	bool flag = isView();
	if (!flag) {
		ChangeState("SEEKWPT");
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0);
	}
	if (VEC3::Distance(player_position->getPosition(), pos) < 4.0f) {
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(1.0f, 0.0f, 0.0f, 1.0);
		ChangeState("IDLWAR");
	}
}

void CAIPatrol_Soldier::IdlWarState(float dt) {
	srand(time(NULL));
	float number = rand() % 100;

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);
	
	if (c_trans->isInLeft(player_position->getPosition())) {
		c_trans->setAngles(yaw + dt, pith);
	}
	else {
		c_trans->setAngles(yaw - dt, pith);
	}



	if (number < 20) {
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(1.0f, 1.0f, 0.0f, 1.0);
		count = 0;
		ChangeState("ORBITLEFT");
	}

	if (number < 40) {
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(1.0f, 1.0f, 0.0f, 1.0);
		count = 0;
		ChangeState("ORBITRIGHT");
	}

	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) > 4.5f) {
		ChangeState("CHASE");
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(0.0f, 1.0f, 0.0f, 1.0);
	}
}

void CAIPatrol_Soldier::OrbitLeftState(float dt) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	

	//se halla vector EP entre Bot y PJ
	float ep = VEC3::Distance(player_position->getPosition(), c_trans->getPosition());
	c_trans->setPosition(VEC3());

	//rotamos x grados
	float yaw, pitch;
	c_trans->getAngles(&yaw, &pitch);
	c_trans->setAngles(yaw - dt, pitch);

	//tranlacion de -EP
	VEC3 front = c_trans->getFront();
	front.Normalize();
	c_trans->setPosition(player_position->getPosition() - front * ep);


	count++;
	if (count > 200) {
		ChangeState("IDLWAR");
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(1.0f, 0.0f, 0.0f, 1.0);
	}
}

void CAIPatrol_Soldier::OrbitRightState(float dt) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//se halla EP
	float ep = VEC3::Distance(player_position->getPosition(), c_trans->getPosition());
	c_trans->setPosition(VEC3());

	//rotamos x grados
	float yaw, pitch;
	c_trans->getAngles(&yaw, &pitch);
	c_trans->setAngles(yaw + dt, pitch);

	//tranlacion de -EP
	VEC3 front = c_trans->getFront();
	front.Normalize();
	c_trans->setPosition(player_position->getPosition() - front * ep);


	count++;
	if (count > 200) {
		ChangeState("IDLWAR");
		TCompRender* render = get<TCompRender>();
		render->color = VEC4(1.0f, 0.0f, 0.0f, 1.0);
	}
}

void CAIPatrol_Soldier::ImpactState(float dt) {
	dbg("IA recibe daño..\n");
	
	if (life <= 0) {
		ChangeState("DEAD");
	}
	TCompRender* render = get<TCompRender>();
	render->color = VEC4(0.0f, 0.0f, 0.0f, 1.0);

	TCompTransform* c_trans = get<TCompTransform>();
	CEntity *e_sender = h_sender;
	if (e_sender != nullptr) {
		TCompTransform* player_position = e_sender->get<TCompTransform>();
		VEC3 direction_player_position = player_position->getFront();

		if (countImpact > 0) {
			VEC3 pos = c_trans->getPosition();
			VEC3 res;
			c_trans->getFront().Normalize(res);
			pos = pos + direction_player_position * dt;
			c_trans->setPosition(pos);
			countImpact--;
		}
		else {
			countImpact = 300;
			TCompRender* render = get<TCompRender>();
			render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0);
			ChangeState("SEEKWPT");
		}
	}
}

void CAIPatrol_Soldier::DeadState(float dt) {
	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();
}

bool CAIPatrol_Soldier::isView() {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distancia = VEC3::Distance(c_trans->getPosition(), player_position->getPosition());
	//Esta en cono
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if ( abs(angle) <= half_cone) {
		//Esta distancia 
		if (distancia <= length_cone) {
			return true;
		}
	}
	return false;
}

void CAIPatrol_Soldier::load(const json& j, TEntityParseContext& ctx) {
	this->Init();
	half_cone = j.value("halfCone", half_cone);
	length_cone = j.value("lengthCone", length_cone);
	hearing_radius = j.value("hearingRadius", hearing_radius);
	speed = j.value("speed", speed);
}

void CAIPatrol_Soldier::debugInMenu() {
	ImGui::Text(state.c_str());
	ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 10.f);
	ImGui::SliderFloat3("Next wpt", &nextPoint.x, -10.0f, 10.0f);
}

void CAIPatrol_Soldier::renderDebug() {
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

void CAIPatrol_Soldier::onDamageInfoMsg(const TMsgDamage& msg) {
	h_sender = msg.h_sender;
	h_bullet = msg.h_bullet;
	damage = msg.intensityDamage;
	life = life - damage;
	TCompLife* c_trans = get<TCompLife>();
	c_trans->setLife(life);
	ChangeState("IMPACT");
	//cambiar estado
}

void CAIPatrol_Soldier::registerMsgs() {
	DECL_MSG(CAIPatrol_Soldier, TMsgDamage, onDamageInfoMsg);
}
