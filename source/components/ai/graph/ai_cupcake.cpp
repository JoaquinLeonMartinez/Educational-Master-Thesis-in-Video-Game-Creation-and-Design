#include "mcv_platform.h"
#include "ai_cupcake.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/john/comp_bullet.h"
#include "components/common/comp_tags.h"
#include "time.h"
#include "stdlib.h"
#include "modules/module_physics.h"
#include "PxPhysicsAPI.h"

using namespace physx;

DECL_OBJ_MANAGER("ai_cupcake", CAICupcake);

void CAICupcake::Init()
{
	//active states
	AddState("SEEKWPT", (statehandler)&CAICupcake::SeekwptState);
	AddState("NEXTWPT", (statehandler)&CAICupcake::NextwptState);
	AddState("CHASE", (statehandler)&CAICupcake::ChaseState);
	AddState("ORBIT", (statehandler)&CAICupcake::OrbitState);
	AddState("NEXTORBIT", (statehandler)&CAICupcake::NextOrbitState);
	AddState("ALIGN", (statehandler)&CAICupcake::AlignState);


	//pasive states
	AddState("IMPACT", (statehandler)&CAICupcake::ImpactState);
	AddState("RECOIL", (statehandler)&CAICupcake::RecoilState);
	AddState("RECOIL2", (statehandler)&CAICupcake::RecoilState2);
	AddState("FALL", (statehandler)&CAICupcake::FallState);
	AddState("BATTERYEFFECT", (statehandler)&CAICupcake::BatteryEfectState);
	AddState("DIVIDE", (statehandler)&CAICupcake::DivideState);
	AddState("DEAD", (statehandler)&CAICupcake::DeadState);

	ChangeState("SEEKWPT");

}

/* ---------------------- States Functions ----------------------*/

/*
Seek next way point
*/
void CAICupcake::SeekwptState(float dt) {
	if (!h_player.isValid()) {
		h_player = getEntityByName("Player");
		return;
	}

	if (init) {
		init = false;
		TCompTransform* c_trans = get<TCompTransform>();
		VEC3 position = c_trans->getPosition();
		//wtp 
		positions.push_back(VEC3(position.x + 5.0f, position.y + 0.0f, position.z + 5.0f)); //estos valores quiza deberian ser por parametro tmb
		positions.push_back(VEC3(position.x + 0.0f, position.y + 0.0f, position.z + 0.0f));
		positions.push_back(VEC3(position.x + 5.0f, position.y + 0.0f, position.z - 5.0f));
		positions.push_back(VEC3(position.x - 5.0f, position.y + 0.0f, position.z + 5.0f));

		nextPoint = positions[wtpIndex];
	}


  //COLOR DEBUG
  TCompRender* render = get<TCompRender>();
  //render->color = VEC4(1.0f, 1.0f, 1.0f, 1.0);

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 dir = VEC3(0.0f, gravity, 0.0f);

	//MOVE
	movement(dt, nextPoint, true);

	//EVALUATE OPTIONS
	float distancia = VEC3::Distance(nextPoint, c_trans->getPosition());
	bool flag = isView(seewWtp2Chase);

	if (flag) {
		if (jumpingCupcake) {  //if is not a jumping cupcake cannot attack directly
			srand(time(NULL));
			float probability = rand() % 100;

			if (probability <= attackProbability) {
				ChangeState("CHASE"); //CHASE == ATTACK
			}
			else {
				generatePoints();
				if (!jumpingCupcake) {
					jump = false;
				}
				ChangeState("ORBIT");
			}
		}
		else {
			generatePoints();
			if (!jumpingCupcake) {
				jump = false;
			}
			ChangeState("ORBIT");
		}
	}
	else if (distancia < wptDistance) {
		ChangeState("NEXTWPT");
	}

}

/*
Change way point
*/
void CAICupcake::NextwptState(float dt) {
	wtpIndex = (wtpIndex + 1) % positions.size();
	nextPoint = positions[wtpIndex];
	ChangeState("SEEKWPT");
}

/*
Change direction to playe
*/
void CAICupcake::AlignState(float dt) {

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);

	//look at the player
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) <= cone_vision) {
		ChangeState("CHASE");
		jump = true;  // you will attack jumping always
	}
	else {
		if (c_trans->isInLeft(player_position->getPosition())) {
			c_trans->setAngles(yaw + dt * twistSpeed, pith);
		}
		else {
			c_trans->setAngles(yaw - dt * twistSpeed, pith);
		}
	}
}

/*
Follow player
*/
void CAICupcake::ChaseState(float dt) {

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();

  //RENDER DEBUG
  TCompRender* render = get<TCompRender>();
  /*if (!jump && isGrounded()) {
    render->color = VEC4(0.0f, 0.0f, 0.0f, 1.0); //Black if you are hide
  }
  else {
    render->color = VEC4(1.0f, 0.0f, 0.0f, 1.0);
  }*/

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 dir = VEC3(0.0f, gravity, 0.0f);

	//MOVE
	movement(dt, player_position->getPosition(), false);

	//EVALUATE OPTIONS
	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) > chase2Seekwtp) {
		ChangeState("SEEKWPT");

		if (!jumpingCupcake) {
			jump = false;
		}
	}
}

/*
Follow next orbit point
*/
void CAICupcake::OrbitState(float dt) {

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 dir = VEC3(0.0f, gravity, 0.0f);

  //RENDER DEBUG
  TCompRender* render = get<TCompRender>();
  /*if (!jump && isGrounded()) {
    render->color = VEC4(0.0f, 0.0f, 0.0f, 1.0); //Black if you are hide
  }
  else {
    render->color = VEC4(1.0f, 1.0f, 1.0f, 1.0);
  }*/

	//MOVE
	movement(dt, nextOrbitPoint, false);

	//EVALUATE OPTIONS
	float distancia = VEC3::Distance(nextOrbitPoint, c_trans->getPosition());
	if (distancia < wptDistance && isGrounded()) {
		ChangeState("NEXTORBIT");
	}
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();

	if (VEC3::Distance(player_position->getPosition(), c_trans->getPosition()) > chase2Seekwtp) {
		ChangeState("SEEKWPT");
		if (!jumpingCupcake) {
			jump = false;
		}
	}
}

/*
Change orbit point
*/
void CAICupcake::NextOrbitState(float dt) {

	generatePoints(); //Update points
	orbitIndex = (orbitIndex + 1) % positions.size();
	nextOrbitPoint = orbitPositions[orbitIndex];

	//EVALUATE OPTIONS
	srand(time(NULL));
	float random = rand() % 100;

	if (random < orbitAttackProb) {
		ChangeState("ALIGN");
	}
	else {
		ChangeState("ORBIT");
	}
}

/*
Decides if you have to die, split or recoil
*/
void CAICupcake::ImpactState(float dt) {
	if (life <= 0) {
		dbg("muere cupcake de generacion %i con numero de divisiones = %i\n", id, num_of_divisions);
		if (num_of_divisions > 0) {
			num_of_divisions--;
			ChangeState("DIVIDE");
		}
		else {
			ChangeState("DEAD");
		}
	}
	else {
		ChangeState("RECOIL2");
	}
}

/*
Generate number of sons ("num_of_sons")
*/
void CAICupcake::DivideState(float dt) {

	TCompTransform* c_trans = get<TCompTransform>();
	TEntityParseContext ctx;
	VEC3 posicion = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
	c_trans->setPosition(posicion);
	//GENERATE SONS
	VEC3 pos = c_trans->getPosition();
	float randX;
	float randZ;
	int i = 0;
	int invertir = 1;
	int msg_id = 0;
	while (i < num_of_sons) {

		invertir = invertir * -1;
		randX = rand() % spawnRange;
		randZ = rand() % spawnRange;
		c_trans->setPosition(VEC3((pos.x + randX * invertir), pos.y, pos.z + (randZ * invertir)));
		ctx.root_transform = *c_trans;

		//ctx.current_entity = CHandle(this).getOwner();
		parseScene("data/prefabs/mini_cupcake.json", ctx);
		i++;

		//send atributes
		/*
		TMsgCupcakeSon msg;
		msg.msg_id = msg_id;
		msg.num_of_divisions = num_of_divisions;
		msg.id = id + 1;
		//msg.num_of_sons = num_of_sons;
		ctx.entities_loaded[0].sendMsg(msg);
		dbg("enviado mensaje %i \n", msg.msg_id, id, num_of_divisions);
		msg_id++;
		*/

	}

	//DIE
	ChangeState("DEAD");
}

/*
Pushes you back
*/
void CAICupcake::RecoilState(float dt) {

  //RENDER DEBUG
  TCompRender* render = get<TCompRender>();
  //render->color = VEC4(1.0f, 1.0f, 1.0f, 1.0);

	TCompTransform* c_trans = get<TCompTransform>();

	VEC3 dir = VEC3();
	//MOVE

	dir = c_trans->getFront() * -stunSpeed;


	if (firstImpact) {//JUMP
		firstImpact = false;
		impulse = dir;
		impulse.y = recoilJumpForce;
	}
	else if (!isGrounded()) {//FALL

							 //gravity
		impulse.y += gravity * dt;
		dir.y += impulse.y;
		dir *= dt;

		//UPDATE

		TCompCollider* c_cc = get<TCompCollider>();
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
		}
	}
	else {//END RECOIL WHEN IS GROUNDED
		ChangeState("ALIGN");
		firstImpact = true;
	}

}

void CAICupcake::FallState(float dt) {


	if (isGrounded()) {
		ChangeState("ALIGN");
	}
	else {
		VEC3 dir = bullet_front * speed;
		impulse.y += gravity * dt;
		dir.y += impulse.y;
		dir *= dt;
		TCompCollider* c_cc = get<TCompCollider>();
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
		}
	}




}

void CAICupcake::RecoilState2(float dt) { //cuando choca algo conmigo

                                          //RENDER DEBUG
  TCompRender* render = get<TCompRender>();
  //render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0);

	TCompTransform* c_trans = get<TCompTransform>();

	VEC3 dir = VEC3();

	if (timerStun > 0) {

		//movimiento de donde venga la bala
		TCompTransform* c_trans = get<TCompTransform>();
		dir = c_trans->getPosition() - bullet_position;

		dir.Normalize();
		//MULTIPLICAR POR LA FUERZA DE IMPACTO
		dir *= bullet_impulse; //OBTENER DE LA BALA EN TEORIA
		dir.y = c_trans->getPosition().y;
		dir *= dt;


		TCompCollider* c_cc = get<TCompCollider>();
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
		}

		timerStun -= dt;
	}
	else {

		timerStun = limitTimeStun;

		ChangeState("FALL"); //ALIGN
	}

}

void CAICupcake::BatteryEfectState(float dt) {

  if (battery_time > 0) {
    battery_time -= dt;
    //moverme hacia battery_position y si estoy en batery position no moverme
    TCompTransform* c_trans = get<TCompTransform>();
    float distancia = VEC3::Distance(battery_position, c_trans->getPosition());
    if (distancia < 3.0) {
      //nothing
      TCompRender* render = get<TCompRender>();
      //render->color = VEC4(0.0f, 1.0f, 1.0f, 1.0);

		}
		else {//go to battery

			VEC3 dir = VEC3();
			dir = battery_position - c_trans->getPosition();
			dir.Normalize();
			//MULTIPLICAR POR LA FUERZA DE IMPACTO
			dir *= batterySpeed; //OBTENER DE LA BALA EN TEORIA
								 //dir.y = c_trans->getPosition().y;
			dir *= dt;

			TCompCollider* c_cc = get<TCompCollider>();
			if (c_cc) {
				c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
			}

		}

		//dbg("battery time = %f \n", battery_time);
	}
	else {
		beingAttracted = false;
		//dbg("entra en align");
		ChangeState("ALIGN");
	}

}

/*
Dead state
*/
void CAICupcake::DeadState(float dt) {
	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();
}

/* ---------------------- Helper Functions  ---------------------*/

/*
Jumping and hide movement in chase, seekWpt and orbit states
*/
void CAICupcake::movement(float dt, VEC3 target, bool seek) {

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 dir = VEC3(0.0f, gravity, 0.0f);

	//MOVE
	if (isGrounded() && (jump || seek)) {//IF GROUNDED AND JUMPING CUPCAKE CANNOT MOVE, JUST TURN
		dir = c_trans->getFront();
	}
	else {
		dir = c_trans->getFront() * speed;
	}

	if (!jump && isGrounded() && !seek) {
		dir = c_trans->getFront() * hideSpeed;
	}

	//GRAVITY
	impulse.y += gravity * dt;
	dir.y += impulse.y;
	dir *= dt;

	//TURN FRONT
	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);

	if (isGrounded()) {

		//TURN:
		float angle = rad2deg(c_trans->getDeltaYawToAimTo(target));
		if (abs(angle) <= cone_vision) {
			view_point = true;
		}
		else {
			if (c_trans->isInLeft(target)) {
				c_trans->setAngles(yaw + dt * twistSpeed, pith);
			}
			else {
				c_trans->setAngles(yaw - dt * twistSpeed, pith);
			}
		}
	}

	// JUMP
	if (isGrounded() && view_point && (jump || seek)) {
		impulse = dir;
		impulse.y = jumpForce;
		view_point = false;
	}

	//UPDATE DIR
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc) {
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	}
}

/*
Generate or update orbit points
*/
void CAICupcake::generatePoints() {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	VEC3 playerCurrentPos = player_position->getPosition();

	orbitPositions.clear();

	//RANDOM GENERATE POINTS //PROBLEMA: A VECES IRA A POR EL PLAYER SIN QUERERLO
	/*
	int range = orbitRadious - minDistanceOrbit;
	int i = 0;
	int inversorX = 1;
	int inversorZ = 1;
	while (i < number_of_orbit_points) {
	float invX = rand() % 2;
	if (invX == 0) {
	inversorX *= -1;
	}
	float invZ = rand() % 2;
	if (invZ == 0) {
	inversorZ *= -1;
	}

	srand(time(NULL));
	float randomX = rand() % range;
	float randomZ = rand() % range;
	randomZ = randomZ + minDistanceOrbit; //PARA TENER DECIMALES
	randomX = randomX + minDistanceOrbit;
	orbitPositions.push_back(VEC3(playerCurrentPos.x + randomX*inversorX, playerCurrentPos.y, playerCurrentPos.z + randomZ*inversorZ));
	i++;
	}
	*/
	orbitPositions.push_back(VEC3(playerCurrentPos.x + orbitRadious, playerCurrentPos.y, playerCurrentPos.z));
	orbitPositions.push_back(VEC3(playerCurrentPos.x, playerCurrentPos.y, playerCurrentPos.z + orbitRadious));
	orbitPositions.push_back(VEC3(playerCurrentPos.x - orbitRadious, playerCurrentPos.y, playerCurrentPos.z));
	orbitPositions.push_back(VEC3(playerCurrentPos.x, playerCurrentPos.y, playerCurrentPos.z - orbitRadious));

	//GET NEAREST POINT
	TCompTransform* c_trans = get<TCompTransform>();
	int nearest = 0;
	float distance;

	for (int i = 0; i < orbitPositions.size(); i++)
	{
		distance = VEC3::Distance(c_trans->getPosition(), orbitPositions[i]);
		if (distance < VEC3::Distance(c_trans->getPosition(), orbitPositions[nearest])) {
			nearest = i;
		}
	}
	//FOLLOW NEAREST POINT
	orbitIndex = nearest;
	nextOrbitPoint = orbitPositions[orbitIndex];
}

bool CAICupcake::isView(float distance) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distancia = VEC3::Distance(c_trans->getPosition(), player_position->getPosition());
	//CONE
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) <= half_cone) {
		//DISTANCE
		if (distancia <= distance) {
			return true;
		}
	}
	return false;
}

bool CAICupcake::isGrounded() {
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 player_pos = c_trans->getPosition();
	player_pos.y += 0.0001f; //So we can detect collision with planes
	PxVec3 origin = VEC3_TO_PXVEC3(player_pos);
	PxVec3 unitDir = VEC3_TO_PXVEC3(VEC3(0, -1, 0));
	PxReal maxDistance = 1.0f;
	PxRaycastBuffer hit;

	bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit);
	if (hit.hasBlock) {
		PxF32 dist = hit.block.distance;
		if (dist <= 0.2f) {
			return true;
		}
	}
	else if (hit.hasAnyHits()) {
		PxF32 dist = hit.touches->distance;
		if (dist <= 0.2f) {
			return true;
		}
	}
	return false;
}

void CAICupcake::load(const json& j, TEntityParseContext& ctx) {
	this->Init();
	half_cone = j.value("halfCone", half_cone);
	length_cone = j.value("lengthCone", length_cone);
	hearing_radius = j.value("hearingRadius", hearing_radius);
	speed = j.value("speed", speed);
	life = j.value("life", life);
	num_of_sons = j.value("num_of_sons", num_of_sons);
	spawnRange = j.value("spawnRange", spawnRange);
	twistSpeed = j.value("twistSpeed", twistSpeed);
	num_of_divisions = j.value("num_of_divisions", num_of_divisions);
	orbitRadious = j.value("orbitRadious", orbitRadious);
	jumpingCupcake = j.value("jumpingCupcake", jumpingCupcake);

	recoilJumpForce = j.value("recoilJumpForce", recoilJumpForce);
	jumpForce = j.value("jumpForce", jumpForce);
	attackProbability = j.value("attackProbability", attackProbability);
	orbitAttackProb = j.value("orbitAttackProb", orbitAttackProb);
	wptDistance = j.value("wptDistance", wptDistance);
	cone_vision = j.value("cone_vision", cone_vision);
	forgetEnemyRange = j.value("forgetEnemyRange", forgetEnemyRange);
	hideSpeed = j.value("hideSpeed", hideSpeed);
	minDistanceOrbit = j.value("minDistanceOrbit", minDistanceOrbit);
	number_of_orbit_points = j.value("number_of_orbit_points", number_of_orbit_points);
	stunSpeed = j.value("stunSpeed", stunSpeed);
	id = j.value("id", id);

}

void CAICupcake::debugInMenu() {
	ImGui::Text(state.c_str());
	ImGui::DragFloat("Life", &life, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 20.f);
	ImGui::DragFloat("hideSpeed", &hideSpeed, 0.1f, 0.f, 30.f);
	ImGui::DragFloat("twistSpeed", &twistSpeed, 0.1f, 0.f, 10.f);
	ImGui::SliderFloat3("Next wpt", &nextPoint.x, -10.0f, 10.0f);
	ImGui::DragFloat("jumpForce", &jumpForce, 0.1f, 0.f, 25.f);
	ImGui::DragFloat("attackProbability", &attackProbability, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("orbitAttackProb", &orbitAttackProb, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("orbitRadious", &orbitRadious, 0.1f, 0.f, 15.f);
	ImGui::DragFloat("stunSpeed", &stunSpeed, 0.1f, 0.f, 20.f);
	ImGui::DragFloat("recoilJumpForce", &recoilJumpForce, 0.1f, 0.f, 25.f);

}

void CAICupcake::renderDebug() {
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

void CAICupcake::onCollision(const TMsgOnContact& msg) { //no se utiliza 
	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	if (source_of_impact) {

		TCompTags* c_tag = source_of_impact->get<TCompTags>();
		if (c_tag) {
			std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);

			if (strcmp("player", tag.c_str()) == 0) {

				if (jump) { //IF YOU ARE HIDE YOU CANNOT SEND DAMAGE
					ChangeState("RECOIL");
					dbg("cupcake collision with player \n");
					//SEND MSG TO PLAYER
					TMsgDamageToPlayer msg;
					msg.h_sender = h_sender;
					msg.h_bullet = CHandle(this).getOwner();
					msg.intensityDamage = damage;
					source_of_impact->sendMsg(msg);
				}
				return;
			}
		}
	}
}

void CAICupcake::onDamage(const TMsgDamageToEnemy& msg) {
	if (jump) { //IF YOU ARE HIDE YOU CANNOT RECEIVE DAMAGE 
		h_sender = msg.h_sender;
		h_bullet = msg.h_bullet;


		life -= msg.intensityDamage;

		CEntity* e_bullet = (CEntity *)h_bullet;
		TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
		bullet_position = bullet_trans->getPosition();
		bullet_front = bullet_trans->getFront();


		if (life < 0) {
			life = 0;
		}
		ChangeState("IMPACT");
	}
}

void CAICupcake::onGravity(const TMsgGravity& msg) {
	//IF YOU ARE HIDE YOU CANNOT RECEIVE DAMAGE
	if (!beingAttracted && msg.time_effect > 0) {
		h_sender = msg.h_sender;
		h_bullet = msg.h_bullet;
		//obtener posicion
		beingAttracted = true;
		CEntity* e_bullet = (CEntity *)h_bullet;
		TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
		battery_position = bullet_trans->getPosition();
		battery_time = msg.time_effect;
		batterySpeed = msg.attractionForce;  
		distanceToBattery = msg.distance;

		ChangeState("BATTERYEFFECT");
	}
	else if (beingAttracted && h_bullet == msg.h_bullet) {

		//obtener posicion
		beingAttracted = true;
		CEntity* e_bullet = (CEntity *)h_bullet;
		TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
		battery_position = bullet_trans->getPosition();

		ChangeState("BATTERYEFFECT");
	}
	


}


void CAICupcake::registerMsgs() {
	DECL_MSG(CAICupcake, TMsgOnContact, onCollision);
	DECL_MSG(CAICupcake, TMsgDamageToEnemy, onDamage);
	DECL_MSG(CAICupcake, TMsgGravity, onGravity);
	// DECL_MSG(CAICupcake, TMsgCupcakeSon, onBorn);

}
