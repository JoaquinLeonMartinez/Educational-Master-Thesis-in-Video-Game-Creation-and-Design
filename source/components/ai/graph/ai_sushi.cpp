#include "mcv_platform.h"
#include "ai_sushi.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_render.h"
#include "components/common/comp_tags.h"
//To be removed/changed, needed for identification
#include "components/common/comp_name.h"

#include "components/john/comp_bullet.h"
#include "modules/module_physics.h"

#include "PxPhysicsAPI.h"


#include "time.h"
#include "stdlib.h"
#include "random"


DECL_OBJ_MANAGER("ai_sushi", CAISushi);
using namespace physx;

std::mt19937 mt(1729);
std::uniform_int_distribution<int> dist(0, 100);

/* --------------------- Helper Functions ---------------------*/
/*DEPRECATED*/
//void CAISushi::LookAt_Player(TCompTransform* c_trans, TCompTransform* player_position, float dt) {
//	//Rotation Control
//	float yaw, pitch;
//	c_trans->getAngles(&yaw, &pitch);
//	if (c_trans->isInLeft(player_position->getPosition())) {
//		c_trans->setAngles(yaw + dt + deg2rad(rotationSpeed), pitch);
//	}
//	else {
//		c_trans->setAngles(yaw - dt - deg2rad(rotationSpeed), pitch);
//	}
//	//End Rotation Control
//}

template <typename T>
void CAISushi::Send_DamageMessage(CEntity* entity, float dmg) {
	//Send damage mesage
	T msg;
	// Who sent this bullet
	msg.h_sender = h_sender;
	// The bullet information �?�?�?
	msg.h_bullet = CHandle(this).getOwner();
	msg.intensityDamage = dmg;
	entity->sendMsg(msg);
}

bool CAISushi::isView() {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distance = Vector3::Distance(c_trans->getPosition(), player_position->getPosition());
	if (!inCombat) {
		//si no estamos en combate, view es dentro del cono y a menos distancia que viewDistance
		//o
		//a menos distancia que hearing_radius
		float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
		return ((abs(angle) <= half_cone) && (distance <= viewDistance)) || distance <= hearing_radius;
	}
	else {
		//si estamos en combate, view es menos distancia que combatViewDistance
		return distance <= combatViewDistance;
	}
	return false;
}

bool CAISushi::isGrounded() {
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

void CAISushi::OnAir(float dt) {
	if (isGrounded() && impulse.y <= 0.0f) {
		ChangeState(last_state);
		impulse.y = 0.0f;
		return;
	}

	//Movement Control
	impulse.y += gravity * dt;
	impulse.x *= 1 - (0.05f * dt);
	impulse.z *= 1 - (0.05f * dt);

	VEC3 dir = impulse * dt;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	//End Movement Control
}
/* -------------------- End Helper Functions --------------------*/

void CAISushi::Init()
{
	AddState("ONAIR", (statehandler)&CAISushi::OnAir);
	AddState("SEEKWPT", (statehandler)&CAISushi::SeekwptState);
	AddState("NEXTWPT", (statehandler)&CAISushi::NextwptState);
	AddState("CHASE", (statehandler)&CAISushi::ChaseState);
	AddState("IDLEWAR", (statehandler)&CAISushi::IdleWarState);
	AddState("BLOCKING", (statehandler)&CAISushi::BlockingState);
	AddState("PREPARECHARGE", (statehandler)&CAISushi::PrepareCharge);
	AddState("PREPAREJUMPCHARGE", (statehandler)&CAISushi::PrepareJumpCharge);
	AddState("CHARGE", (statehandler)&CAISushi::ChargeState);
	AddState("JUMPCHARGE", (statehandler)&CAISushi::JumpChargeState);
	AddState("SALUTE", (statehandler)&CAISushi::SaluteState);
	AddState("ORBITLEFT", (statehandler)&CAISushi::OrbitLeftState);
	AddState("ORBITRIGHT", (statehandler)&CAISushi::OrbitRightState);
	AddState("IMPACT", (statehandler)&CAISushi::ImpactState);
	AddState("MELEE1", (statehandler)&CAISushi::Melee1State);
	AddState("MELEE2", (statehandler)&CAISushi::Melee2State);
	AddState("MELEE3", (statehandler)&CAISushi::Melee3State);
	AddState("DEAD", (statehandler)&CAISushi::DeadState);
	AddState("BATTERYEFFECT", (statehandler)&CAISushi::BatteryEffectState);


	ChangeState("SEEKWPT");
}


void CAISushi::NextwptState(float dt) {
	last_state = state;
	TCompTransform* c_trans = get<TCompTransform>();
	wtpIndex = (wtpIndex + 1) % positions.size();

	//dbg("Next Waypoint is %f, %f, %f\n", nextPoint.x, nextPoint.y, nextPoint.z);
	ChangeState("SEEKWPT");
	/*
	TCompRender* render = get<TCompRender>();
	render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0);
	*/
}

void CAISushi::SeekwptState(float dt) {
	last_state = state;
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

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	currentPosition = c_trans->getPosition();

	//Gravity control
	Vector3 dir = VEC3(0.f, gravity, 0.f);
	dir = c_trans->getFront() * patrolSpeed;

	if (!isGrounded()) {
		impulse = dir;
		ChangeState("ONAIR");
		return;
	}

	dir *= dt;

	//MOVE PLAYER
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());

	if (Vector3::Distance(nextPoint, c_trans->getPosition()) < distanceCheckThreshold) {
		ChangeState("NEXTWPT");
	}
	else {
		float yaw, pitch;
		c_trans->getAngles(&yaw, &pitch);
		if (c_trans->isInLeft(nextPoint)) {
			c_trans->setAngles(yaw + dt, pitch);
		}
		else {
			c_trans->setAngles(yaw - dt, pitch);
		}
	}

	//if the player is visible
	if (isView()) {
		inCombat = true;
		TCompName* cname = get<TCompName>();
		//dbg("%s sees the player\n", cname->getName());
		//Three options: Jump Charge, Charge or Salute
		int dice = 0;
		if (randomTimer <= 0) {
			dice = dist(mt);
			randomTimer = randomDelay;
			if (dice < chargeProbability) {//dice < chargeProbability
				dbg("%s activated CHARGE from SeekWpt with dice = %d, which is less than %d\n", cname->getName(), dice, chargeProbability);
				ChangeState("PREPARECHARGE");
				//Make sure the timer has the right value
				chargeTimer = chargeDelay;
				return;
			}
			dice = dist(mt);
			if (dice < saluteProbability) {//dice < saluteProbability
				dbg("%s activated SALUTE from SeekWpt with dice = %d, which is less than %d\n", cname->getName(), dice, saluteProbability);
				ChangeState("SALUTE");
				return;
			}
			dice = dist(mt);
			if (dice < jumpChargeProbability) {//dice < jumpChargeProbability
				dbg("%s activated PREPAREJUMPCHARGE from SeekWpt with dice = %d, which is less than %d\n", cname->getName(), dice, chargeProbability);
				ChangeState("PREPAREJUMPCHARGE");
				return;
			}
		}
		else {
			randomTimer -= dt;
		}

		//Fallback mechanism
		dbg("%s activated CHASE from SeekWpt because everything else failed\n", cname->getName());
		ChangeState("CHASE");
	}
}

/*
Run after the player, checking for dice rolls in order to execute special attacks while on the move
*/
void CAISushi::ChaseState(float dt) {
	last_state = state;
	TCompName* cname = get<TCompName>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//Movement Control
	Vector3 dir = Vector3(0.f, gravity, 0.f);
	dir = c_trans->getFront() * chaseSpeed;

	if (!isGrounded()) {
		last_state = state;
		impulse = dir;
		ChangeState("ONAIR");
		return;
	}

	dir *= dt;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	//End Movement Control

	//Rotation Control
	//LookAt_Player(c_trans, player_position, dt);
	c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	//End Rotation Control

	//State Change Conditions
	if (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) < meleeDistance) {
		dbg("%s reaches melee distance and starts IDLEWAR\n", cname->getName());
		ChangeState("IDLEWAR");
		return;
	}

	//Random chance of activating CHARGE or JUMPCHARGE while CHASE is active


	if (randomTimer <= 0) {
		int dice = dist(mt);
		randomTimer = randomDelay;
		if (dice < chargeProbability) {//dice < chargeProbability
			dbg("%s activated PREPARECHARGE from CHASE with dice = %d, which is less than %d\n", cname->getName(), dice, chargeProbability);
			ChangeState("PREPARECHARGE");
			chargeTimer = chargeDelay;
			if (hasBlocked) hasBlocked = !hasBlocked;
			return;
		}
		dice = dist(mt);
		if (dice < jumpChargeProbability) {//dice < jumpChargeProbability
			dbg("%s activated PREPAREJUMPCHARGE from CHASE with dice = %d, which is less than %d\n", cname->getName(), dice, chargeProbability);
			ChangeState("PREPAREJUMPCHARGE");
			if (hasBlocked) hasBlocked = !hasBlocked;
			return;
		}
	}
	else {
		randomTimer -= dt;
	}	

	if (inCombat && !isView()) {
		dbg("%s forgets the player and goes back to SEEKWPT\n", cname->getName());
		inCombat = false;
		ChangeState("SEEKWPT");
	}
}

/*
Look at the player, check dice rolls in order to make special attacks, attack with melee should everything fail the roll
*/
void CAISushi::IdleWarState(float dt) {
	last_state = state;
	TCompName* cname = get<TCompName>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//Exit condition
	if (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) > meleeDistance) {
		ChangeState("CHASE");
		return;
	}

	//Rotation Control
	//LookAt_Player(c_trans, player_position, dt);
	c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	//Rotation Control

	//Gravity control
	if (!isGrounded()) {
		ChangeState("ONAIR");
		return;
	}

	if (randomTimer <= 0) {
		int dice = dist(mt);
		randomTimer = randomDelay;
		if (dice < chargeProbability) {//dice < chargeProbability
			dbg("%s activated CHARGE from IDLEWAR with dice = %d, which is less than %d\n", cname->getName(), dice, chargeProbability);
			ChangeState("PREPARECHARGE");
			if (hasBlocked) hasBlocked = !hasBlocked;
			return;
		}
		dice = dist(mt);
		if (dice < jumpChargeProbability) {//dice < jumpChargeProbability
			dbg("%s activated PREPAREJUMPCHARGE from IDLEWAR with dice = %d, which is less than %d\n", cname->getName(), dice, jumpChargeProbability);
			ChangeState("PREPAREJUMPCHARGE");
			if (hasBlocked) hasBlocked = !hasBlocked;
			return;
		}
		dice = dist(mt);
		if (dice < blockProbability && !hasBlocked) {//dice < blockProbability
			dbg("%s activated BLOCKING from IDLEWAR with dice = %d, which is less than %d\n", cname->getName(), dice, blockProbability);
			ChangeState("BLOCKING");
			isBlocking = true;
			hasBlocked = true;
			return;
		}
	}
	else {
		randomTimer -= dt;
	}

	dbg("%s activated MELEE1 from IDLEWAR because everything else failed\n", cname->getName());
	ChangeState("MELEE1");
	if (hasBlocked) hasBlocked = !hasBlocked;
}

/*
Jump in the air, after a delay change to JUMPCHARGE, landing in the position of the player
*/
void CAISushi::PrepareJumpCharge(float dt) {
	last_state = state;
	TCompName* cname = get<TCompName>();
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	//Rotation Control
	//LookAt_Player(c_trans, playerTrans, dt);
	c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	//End Rotation Control
	//Get desired point
	if (jumpPosition == VEC3().Zero) {
		jumpPosition = c_trans->getPosition();
		jumpPosition.y += jumpHeight;
	}
	//Get direction
	VEC3 dir = VEC3();
	dir = c_trans->getUp() * jumpSpeed;
	dir *= dt;

	TCompCollider* comp_collider = get<TCompCollider>();
	if (!comp_collider || !comp_collider->controller)
		return;

	comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());

	if (c_trans->getPosition().y >= jumpPosition.y) {
		dbg("%s activated JUMPCHARGE from PREPAREJUMPCHARGE\n", cname->getName());
		ChangeState("JUMPCHARGE");
		jumpPosition = VEC3().Zero;
		chargeObjective = p_trans->getPosition();
	}
}

/*
Land in the position of the player
*/
void CAISushi::JumpChargeState(float dt) {
	last_state = state;
	TCompName* cname = get<TCompName>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* c_trans = get<TCompTransform>();
	TCompTransform* player_position = e_player->get<TCompTransform>();
	//Move towards that direction
	VEC3 dir = (chargeObjective - c_trans->getPosition()) * jumpChargeSpeed;
	dir *= dt;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc) {
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	}
	//End Charge

	//Exit condition
	if (Vector3::Distance(chargeObjective, c_trans->getPosition()) < distanceCheckThreshold) {
		dbg("%s activated CHASE from JUMPCHARGE\n", cname->getName());
		ChangeState("CHASE");
		chargeObjective = Vector3().Zero;
	}
}

/*
Play charge preparation animation while acquiring charge parameters
After a delay, change to CHARGE
*/
void CAISushi::PrepareCharge(float dt) {
	last_state = state;
	//Gravity control
	if (!isGrounded()) {
		ChangeState("ONAIR");
		return;
	}
	//PREPARING CHARGE
	chargeTimer -= dt;

	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	//Rotation Control
	//LookAt_Player(c_trans, playerTrans, dt);
	c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	//End Rotation Control

	if (chargeTimer <= 0.0f) {
		VEC3 player_pos = p_trans->getPosition();
		VEC3 my_pos = c_trans->getPosition();
		chargeObjective = player_pos - my_pos;
		chargeObjective.y = 0.0f;
		//To normalize or not to normalize, that is the question
		chargeObjective.Normalize();
		chargeElapsed = 0.0f;
		chargeTimer = chargeDelay;
		chargePoint = c_trans->getPosition() + (c_trans->getFront() * chargeSpeed * chargeDuration);
		TCompName* cname = get<TCompName>();
		dbg("%s activated CHARGE from PREPARECHARGE\n", cname->getName());
		ChangeState("CHARGE");
	}
}

/*
Advance a set distance in the direction of the player
*/
void CAISushi::ChargeState(float dt) {
	last_state = state;
	if (chargeElapsed >= chargeDuration) {
		ChangeState("CHASE");
		return;
	}

	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* c_trans = get<TCompTransform>();
	TCompTransform* player_position = e_player->get<TCompTransform>();

	//Move towards that direction
	VEC3 dir = chargeObjective * chargeSpeed * dt;
	impulse = dir;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc) {
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	}

	chargeElapsed += dt;
}

/*
Play salute animation and proceed to chase player
*/
void CAISushi::SaluteState(float dt) {
	last_state = state;
	TCompName* cname = get<TCompName>();
	if (saluteElapsed < saluteDuration) {
		dbg("%s executes Salute\n", cname->getName());
		TCompTransform* c_trans = get<TCompTransform>();
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		//LookAt_Player(c_trans, player_position, dt);
		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
		//Salute code
		saluteElapsed += dt;
	}
	else {
		dbg("%s begins chasing the player\n", cname->getName());
		ChangeState("CHASE");
	}
}

void CAISushi::Melee1State(float dt) {
	last_state = state;
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	//Exit condition
	if (Vector3::Distance(player_position->getPosition(), c_trans->getPosition()) > meleeDistance) {
		ChangeState("CHASE");
		return;
	}
	//Gravity control
	if (!isGrounded()) {
		ChangeState("ONAIR");
		return;
	}
	//Rotation Control
	//LookAt_Player(c_trans, player_position, dt);
	c_trans->rotateTowards(player_position->getPosition(), rotationSpeed, dt);
	//End Rotation Control
	if (meleeTimer <= 0) {
		TCompName* cname = get<TCompName>();
		TCompCollider* comp_collider = get<TCompCollider>();
		//Create a collider sphere where we want to detect collision
		//Old code
		/*
		PxSphereGeometry geometry(1.0); //NO HARDCODEADO
		Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
		PxF32 attackHeight = comp_collider->controller->getHeight() / 2;
		damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;
		PxVec3 pos = VEC3_TO_PXVEC3(damageOrigin);
		PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());

		PxOverlapBuffer hit;
		PxTransform shapePose = PxTransform(pos, ori);    // [in] initial shape pose (at distance=0)
														  //Evaluate collisions
		bool res = EnginePhysics.gScene->overlap(geometry, shapePose, hit, PxQueryFilterData(PxQueryFlag::eANY_HIT | PxQueryFlag::eDYNAMIC));

		//If there were any hits
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
							dbg("%s hits MELEE1\n", cname->getName());
							//Send damage mesage
							Send_DamageMessage<TMsgDamageToPlayer>(entityContact, melee1Damage);
							//Check for MELEE2

							int dice = dist(mt);
							if (dice < meleeComboProbability) {
								dbg("%s activated MELEE2 from MELEE1 with dice = %d, which is less than %d\n", cname->getName(), dice, meleeComboProbability);
								ChangeState("MELEE2");
								//Combo: Next attack is faster than normal
								meleeTimer = meleeDelay / 2;
								return;
							}
							else {
								meleeTimer = meleeDelay;
							}
						}
						else {
							dbg("%s misses MELEE1, goes back to IDLEWAR\n", cname->getName());
							ChangeState("IDLEWAR");
						}
					}
				}
			}
		}
		else {
			dbg("%s misses MELEE1, goes back to IDLEWAR\n", cname->getName());
			ChangeState("IDLEWAR");
		}*/
		//New Code
		PxSphereGeometry geometry(1.0); //NO HARDCODEADO

		Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
		PxF32 attackHeight = comp_collider->controller->getHeight() / 2;
		damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;

		physx::PxFilterData pxFilterData;
		pxFilterData.word0 = EnginePhysics.Player;
		physx::PxQueryFilterData PxPlayerFilterData;
		PxPlayerFilterData.data = pxFilterData;
		PxPlayerFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
		std::vector<physx::PxOverlapHit> hits;
		if (EnginePhysics.Overlap(geometry, damageOrigin, hits, PxPlayerFilterData)) {
			for (int i = 0; i < hits.size(); i++) {
				CHandle hitCollider;
				hitCollider.fromVoidPtr(hits[i].actor->userData);
				if (hitCollider.isValid()) {
					CHandle player = hitCollider.getOwner();
					if (player == h_player) {
						dbg("%s hits MELEE1\n", cname->getName());
						//Send damage mesage
						CEntity* entityContact = hitCollider.getOwner();
						Send_DamageMessage<TMsgDamageToPlayer>(entityContact, melee1Damage);
						//Check for MELEE2

						int dice = dist(mt);
						if (dice < meleeComboProbability) {
							dbg("%s activated MELEE2 from MELEE1 with dice = %d, which is less than %d\n", cname->getName(), dice, meleeComboProbability);
							ChangeState("MELEE2");
							//Combo: Next attack is faster than normal
							meleeTimer = meleeDelay * meleeComboDelayReducedFactor;
							return;
						}
						else {
							meleeTimer = meleeDelay;
						}
					}
				}
			}
		}
		else {
			dbg("%s misses MELEE1, goes back to IDLEWAR\n", cname->getName());
			ChangeState("IDLEWAR");
		}
	}
	else {
		meleeTimer -= dt;
	}
}

void CAISushi::Melee2State(float dt) {
	last_state = state;
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	//Exit condition
	if (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) > meleeDistance) {
		ChangeState("CHASE");
		return;
	}
	//Gravity control
	if (!isGrounded()) {
		ChangeState("ONAIR");
		return;
	}
	//Rotation Control
	//LookAt_Player(c_trans, player_position, dt);
	c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	//End Rotation Control
	if (meleeTimer <= 0) {
		TCompName* cname = get<TCompName>();
		TCompCollider* comp_collider = get<TCompCollider>();
		TCompTransform* c_trans = get<TCompTransform>();
		//Create a collider sphere where we want to detect collision
		PxSphereGeometry geometry(1.0); //NO HARDCODEADO

		Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
		PxF32 attackHeight = comp_collider->controller->getHeight() / 2;
		damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;

		physx::PxFilterData pxFilterData;
		pxFilterData.word0 = EnginePhysics.Player;
		physx::PxQueryFilterData PxPlayerFilterData;
		PxPlayerFilterData.data = pxFilterData;
		PxPlayerFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
		std::vector<physx::PxOverlapHit> hits;
		if (EnginePhysics.Overlap(geometry, damageOrigin, hits, PxPlayerFilterData)) {
			for (int i = 0; i < hits.size(); i++) {
				CHandle hitCollider;
				hitCollider.fromVoidPtr(hits[i].actor->userData);
				if (hitCollider.isValid()) {
					CHandle player = hitCollider.getOwner();
					if (player == h_player) {
						dbg("%s hits MELEE2\n", cname->getName());
						//Send damage mesage
						CEntity* entityContact = hitCollider.getOwner();
						Send_DamageMessage<TMsgDamageToPlayer>(entityContact, melee2Damage);
						//Check for MELEE3

						int dice = dist(mt);
						if (dice < meleeComboProbability) {
							dbg("%s activated MELEE3 from MELEE2 with dice = %d, which is less than %d\n", cname->getName(), dice, meleeComboProbability);
							ChangeState("MELEE3");
							//Combo: Next attack is faster than normal
							meleeTimer = meleeDelay * meleeComboDelayReducedFactor;
							return;
						}
						else {
							meleeTimer = meleeDelay;
						}
					}
				}
			}
		}
		else {
			dbg("%s misses MELEE2, goes back to IDLEWAR\n", cname->getName());
			ChangeState("IDLEWAR");
		}
	}
	else {
		meleeTimer -= dt;
	}
}

void CAISushi::Melee3State(float dt) {
	last_state = state;
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	//Exit condition
	if (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) > meleeDistance) {
		ChangeState("CHASE");
		return;
	}
	//Gravity control
	if (!isGrounded()) {
		ChangeState("ONAIR");
		return;
	}
	//Rotation Control
	//LookAt_Player(c_trans, player_position, dt);
	c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	//End Rotation Control
	if (meleeTimer <= 0) {
		TCompName* cname = get<TCompName>();
		TCompCollider* comp_collider = get<TCompCollider>();
		TCompTransform* c_trans = get<TCompTransform>();
		//Create a collider sphere where we want to detect collision
		PxSphereGeometry geometry(1.0); //NO HARDCODEADO

		Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
		PxF32 attackHeight = comp_collider->controller->getHeight() / 2;
		damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;

		physx::PxFilterData pxFilterData;
		pxFilterData.word0 = EnginePhysics.Player;
		physx::PxQueryFilterData PxPlayerFilterData;
		PxPlayerFilterData.data = pxFilterData;
		PxPlayerFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
		std::vector<physx::PxOverlapHit> hits;
		if (EnginePhysics.Overlap(geometry, damageOrigin, hits, PxPlayerFilterData)) {
			for (int i = 0; i < hits.size(); i++) {
				CHandle hitCollider;
				hitCollider.fromVoidPtr(hits[i].actor->userData);
				if (hitCollider.isValid()) {
					CHandle player = hitCollider.getOwner();
					if (player == h_player) {
						dbg("%s hits MELEE3\n", cname->getName());
						//Send damage mesage
						CEntity* entityContact = hitCollider.getOwner();
						Send_DamageMessage<TMsgDamageToPlayer>(entityContact, melee3Damage);
					}
				}
			}
		}
		else {
			dbg("%s misses MELEE3, goes back to IDLEWAR\n", cname->getName());
			ChangeState("IDLEWAR");
		}
	}
	else {
		meleeTimer -= dt;
	}
}

void CAISushi::BlockingState(float dt) {
	last_state = state;
	//Gravity control
	if (!isGrounded()) {
		ChangeState("ONAIR");
		return;
	}
	if (blockRemaining > 0 && isBlocking) {
		int dice;
		if (randomTimer <= 0) {
			dice = dist(mt);
			randomTimer = randomDelay;
			if (dice < orbitProbability) {//dice < orbitProbability
				if (dice % 2) {//dice % 2
					TCompName* cname = get<TCompName>();
					dbg("%s activated ORBITLEFT from BLOCKING with dice = %d, which is less than %d\n", cname->getName(), dice, orbitProbability);
					ChangeState("ORBITLEFT");
					return;
				}
				else {
					TCompName* cname = get<TCompName>();
					dbg("%s activated ORBITRIGHT from BLOCKING with dice = %d, which is less than %d\n", cname->getName(), dice, orbitProbability);
					ChangeState("ORBITRIGHT");
					return;
				}
			}
		}
		else {
			randomTimer -= dt;
		}		
		blockRemaining -= dt;
	}
	else {
		isBlocking = false;
		blockRemaining = blockDuration;
		ChangeState("IDLEWAR");
		TCompName* cname = get<TCompName>();
		dbg("%s finished BLOCKING and went back to IDLEWAR \n", cname->getName());
	}
}

void CAISushi::OrbitLeftState(float dt) {
	last_state = state;
	if (blockRemaining > 0 && isBlocking) {
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		TCompTransform* c_trans = get<TCompTransform>();
		Vector3 p_pos = p_trans->getPosition();
		Vector3 e_pos = c_trans->getPosition();

		float s = sin(dt);
		float c = cos(dt);

		// translate point back to origin:
		e_pos.x -= p_pos.x;
		e_pos.z -= p_pos.z;

		// rotate point
		float xnew = e_pos.x * c - e_pos.z * s;
		float ynew = e_pos.x * s + e_pos.z * c;

		// translate point back:
		e_pos.x = xnew + p_pos.x;
		e_pos.z = ynew + p_pos.z;

		//Rotation Control
		//LookAt_Player(c_trans, player_position, dt);
		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
		//End Rotation Control		

		VEC3 dir = e_pos - c_trans->getPosition();
		dir.Normalize();
		dir = dir * orbitSpeed;

		if (!isGrounded()) {
			last_state = state;
			impulse = dir;
			ChangeState("ONAIR");
			return;
		}

		dir *= dt;

		TCompCollider* c_cc = get<TCompCollider>();
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
		}

		blockRemaining -= dt;
	}
	else {
		isBlocking = false;
		blockRemaining = blockDuration;
		TCompName* cname = get<TCompName>();
		dbg("%s finished BLOCKING and ORBITLEFT and went back to IDLEWAR \n", cname->getName());
		ChangeState("IDLEWAR");
	}
}

void CAISushi::OrbitRightState(float dt) {
	last_state = state;
	if (blockRemaining > 0 && isBlocking) {
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		TCompTransform* c_trans = get<TCompTransform>();
		Vector3 p_pos = p_trans->getPosition();
		Vector3 e_pos = c_trans->getPosition();

		float s = sin(-dt);
		float c = cos(-dt);

		// translate point back to origin:
		e_pos.x -= p_pos.x;
		e_pos.z -= p_pos.z;

		// rotate point
		float xnew = e_pos.x * c - e_pos.z * s;
		float ynew = e_pos.x * s + e_pos.z * c;

		// translate point back:
		e_pos.x = xnew + p_pos.x;
		e_pos.z = ynew + p_pos.z;

		//Rotation Control
		//LookAt_Player(c_trans, player_position, dt);
		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
		//End Rotation Control

		//tranlacion de -EP
		VEC3 dir = e_pos - c_trans->getPosition();
		dir.Normalize();
		dir = dir * orbitSpeed;

		if (!isGrounded()) {
			last_state = state;
			impulse = dir;
			ChangeState("ONAIR");
			return;
		}

		dir *= dt;

		TCompCollider* c_cc = get<TCompCollider>();
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
		}

		blockRemaining -= dt;
	}
	else {
		isBlocking = false;
		blockRemaining = blockDuration;
		TCompName* cname = get<TCompName>();
		dbg("%s finished BLOCKING and ORBITRIGHT and went back to IDLEWAR \n", cname->getName());
		ChangeState("IDLEWAR");
	}
}

void CAISushi::BatteryEffectState(float dt) {

	if (battery_time > 0) {
		battery_time -= dt;
		//moverme hacia battery_position y si estoy en batery position no moverme
		TCompTransform* c_trans = get<TCompTransform>();
		float distancia = VEC3::Distance(battery_position, c_trans->getPosition());
		if (distancia < distanceToBattery) {
			//TCompRender* render = get<TCompRender>();
			//render->color = VEC4(0.0f, 1.0f, 1.0f, 1.0);

		}
		else {//go to battery

			VEC3 dir = VEC3();
			dir = battery_position - c_trans->getPosition();
			dir.Normalize();
			//MULTIPLICAR POR LA FUERZA DE IMPACTO
			dir *= attractionForce; //OBTENER DE LA BALA EN TEORIA
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
		dbg("BATTERY ends, sushi goes back to CHASE\n");
		ChangeState("CHASE");
	}

}

void CAISushi::ImpactState(float dt) {
	TCompName* cname = get<TCompName>();
	dbg("%s receives an impact\n", cname->getName());

	if (life <= 0.f) {
		ChangeState("DEAD");
	}
	//TCompRender* render = get<TCompRender>();
	//render->color = VEC4(1.0f, 1.0f, 1.0f, 1.0);

	if (damageStunTimer > 0) {
		TCompTransform* c_trans = get<TCompTransform>();

		//Rotate towards player
		//LookAt_Player(c_trans, player_position, dt);
		//Move away from player for damageStunDuration seconds
		float impactForce = 5.f;//NO HARDCODEADO -> DEBER�A VENIR DE LA BALA
		VEC3 dir = c_trans->getPosition() - damageSource;
		dir.Normalize();
		dir *= impactForce;
		dir *= dt;
		TCompCollider* c_cc = get<TCompCollider>();
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
		}
		damageStunTimer -= dt;
	}
	else {
		damageSource = Vector3();
		damageStunTimer = damageStunDuration;
		//TCompRender* render = get<TCompRender>();
		//render->color = VEC4(0.0f, 0.0f, 1.0f, 1.0);
		if (last_state == "SEEKWPT" || last_state == "NEXTWPT") {
			inCombat = true;
			ChangeState("CHASE");
		}
		else {
			ChangeState(last_state);
		}
	}
}

void CAISushi::DeadState(float dt) {
	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();
}

void CAISushi::load(const json& j, TEntityParseContext& ctx) {
	this->Init();
	//Pathing
	//nWaypoints = j.value("nWaypoints", nWaypoints);
	//auto k = j["waypoints"];
	//End Pathing

	//View Ranges
	half_cone = j.value("halfCone", half_cone);
	hearing_radius = j.value("hearingRadius", hearing_radius);
	combatViewDistance = j.value("combatViewDistance", combatViewDistance);
	viewDistance = j.value("viewDistance", viewDistance);
	//End View Ranges

	//Core Values
	life = j.value("life", life);
	patrolSpeed = j.value("patrolSpeed", patrolSpeed);
	chaseSpeed = j.value("chaseSpeed", chaseSpeed);
	rotationSpeed = j.value("rotationSpeed", rotationSpeed);
	damageStunDuration = j.value("damageStunDuration", damageStunDuration);
	damageStunBackwardsSpeed = j.value("damageStunBackwardsSpeed", damageStunBackwardsSpeed);
	distanceCheckThreshold = j.value("distanceCheckThreshold", distanceCheckThreshold);
	//End Core Values

	//Melee Values
	meleeComboProbability = j.value("meleeComboProbability", meleeComboProbability);
	melee1Damage = j.value("melee1Damage", melee1Damage);
	melee2Damage = j.value("melee2Damage", melee2Damage);
	melee3Damage = j.value("melee3Damage", melee3Damage);
	meleeDistance = j.value("meleeDistance", meleeDistance);
	meleeDelay = j.value("meleeDelay", meleeDelay);
	meleeComboDelayReducedFactor = j.value("meleeComboDelayReducedFactor", meleeComboDelayReducedFactor);
	//End Melee Values

	//Charge Values
	chargeProbability = j.value("chargeProbability", chargeProbability);
	chargeSpeed = j.value("chargeSpeed", chargeSpeed);
	chargeDamage = j.value("chargeDamage", chargeDamage);
	chargeDelay = j.value("chargeDelay", chargeDelay);
	chargeDuration = j.value("chargeDuration", chargeDuration);
	//End Charge Values

	//Jump Charge Values
	jumpChargeProbability = j.value("jumpChargeProbability", jumpChargeProbability);
	jumpSpeed = j.value("jumpSpeed", jumpSpeed);
	jumpHeight = j.value("jumpHeight", jumpHeight);
	jumpChargeSpeed = j.value("jumpChargeSpeed", jumpChargeSpeed);
	jumpChargeDamage = j.value("jumpChargeDamage", jumpChargeDamage);
	//End Jump Charge Values

	//Salute Values
	saluteProbability = j.value("saluteProbability", saluteProbability);
	saluteDuration = j.value("saluteDuration", saluteDuration);
	//End Salute Values

	//Blocking Values
	blockProbability = j.value("blockProbability", blockProbability);
	blockDuration = j.value("blockDuration", blockDuration);
	//End Blocking Values

	//Orbit Values
	orbitProbability = j.value("orbitProbability", orbitProbability);
	orbitSpeed = j.value("orbitSpeed", orbitSpeed);
	//End Orbit Values
}

void CAISushi::debugInMenu() {
	ImGui::LabelText("State", "%s", state.c_str());

	//View Ranges
	//ImGui::TreeNode("View Ranges");
	ImGui::DragFloat("half_cone", &half_cone, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("hearing_radius", &hearing_radius, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("combatViewDistance", &combatViewDistance, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("viewDistance", &viewDistance, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End View Ranges

	//Core Values
	//ImGui::TreeNode("Core Values");
	ImGui::DragFloat("life", &life, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("patrolSpeed", &patrolSpeed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("chaseSpeed", &chaseSpeed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("rotationSpeed", &rotationSpeed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("damageStunDuration", &damageStunDuration, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("damageStunTimer", &damageStunTimer, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("damageStunBackwardsSpeed", &damageStunBackwardsSpeed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("distanceCheckThreshold", &distanceCheckThreshold, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End Core Values

	//Melee Values
	//ImGui::TreeNode("Core Values");
	ImGui::DragInt("meleeComboProbability", &meleeComboProbability, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("melee1Damage", &melee1Damage, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("melee2Damage", &melee2Damage, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("melee3Damage", &melee3Damage, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("meleeDistance", &meleeDistance, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("meleeDelay", &meleeDelay, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("meleeTimer", &meleeTimer, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End Melee Values

	//Charge Values
	//ImGui::TreeNode("Charge Values");
	ImGui::DragInt("chargeProbability", &chargeProbability, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("chargeSpeed", &chargeSpeed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("chargeDamage", &chargeDamage, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("chargeDelay", &chargeDelay, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("chargeDuration", &chargeDuration, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("chargeElapsed", &chargeElapsed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("chargeTimer", &chargeTimer, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End Charge Values

	//Jump Charge Values
	//ImGui::TreeNode("Jump Charge Values");
	ImGui::DragInt("jumpChargeProbability", &jumpChargeProbability, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("jumpSpeed", &jumpSpeed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("jumpHeight", &jumpHeight, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("jumpChargeSpeed", &jumpChargeSpeed, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("jumpChargeDamage", &jumpChargeDamage, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End Jump Charge Values


	//Salute Values
	//ImGui::TreeNode("Salute Values");
	ImGui::DragInt("saluteProbability", &saluteProbability, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("saluteDuration", &saluteDuration, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("saluteElapsed", &saluteElapsed, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End Salute Values

	//Blocking Values
	//ImGui::TreeNode("Blocking Values");
	ImGui::Checkbox("hasBlocked", &hasBlocked);
	ImGui::Checkbox("isBlocking", &hasBlocked);
	ImGui::DragInt("blockProbability", &blockProbability, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("blockDuration", &blockDuration, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("blockRemaining", &blockRemaining, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End Blocking Values

	//Orbit Values
	//ImGui::TreeNode("Orbit Values");
	ImGui::DragInt("orbitProbability", &orbitProbability, 0.1f, 0.f, 100.f);
	ImGui::DragFloat("orbitSpeed", &orbitSpeed, 0.1f, 0.f, 100.f);
	//ImGui::TreePop();
	//End Orbit Values

	//Flags
	//ImGui::TreeNode("Flags");
	ImGui::LabelText("Last State", "%s", last_state.c_str());
	ImGui::Checkbox("inCombat", &inCombat);
	//ImGui::TreePop();
	//End Flags
}

void CAISushi::renderDebug() {
	TCompTransform* c_trans = get<TCompTransform>();
	TCompRender* c_render = get<TCompRender>();
	Vector3 front = c_trans->getFront();
	Vector3 pos = c_trans->getPosition();
	float angle = deg2rad(half_cone);

	//Create a rotation matrix with the angle
	Matrix aux_cone_1 = Matrix::CreateRotationY(angle);
	Matrix aux_cone_2 = Matrix::CreateRotationY(-angle);

	//Create two vectors to store the result
	Vector3 half_cone_1, half_cone_2;

	//We rotate the vector "front" with the matrix "aux_cone_X" into "half_cone_X"
	Vector3::Transform(front, aux_cone_1, half_cone_1);
	Vector3::Transform(front, aux_cone_2, half_cone_2);
	half_cone_1.Normalize();
	half_cone_2.Normalize();

	if (!inCombat) {
		drawCircle(pos, hearing_radius, c_render->color);
		drawLine(pos, pos + half_cone_1 * viewDistance, c_render->color);
		drawLine(pos, pos + half_cone_2 * viewDistance, c_render->color);
		drawLine(pos + half_cone_1 * viewDistance, pos + half_cone_2 * viewDistance, c_render->color);
	}
	else {
		if (state == "CHARGE") {
			drawCircle(chargePoint, meleeDistance, VEC4(1, 0, 0, 1));
			drawLine(pos, chargePoint, VEC4(1, 0, 0, 1));
		}
		else if (state == "JUMPCHARGE") {
			drawCircle(chargeObjective, meleeDistance, VEC4(1, 0, 0, 1));
			drawLine(pos, chargeObjective, VEC4(1, 0, 0, 1));
		}
		else if (state == "MELEE1" || state == "MELEE2" || state == "MELEE3") {
			drawCircle(pos, meleeDistance, VEC4(1, 0, 0, 1));
		}
		else {
			drawCircle(pos, meleeDistance, VEC4(1, 0, 0, 1));
			drawCircle(pos, hearing_radius, VEC4(1, 0, 0, 1));
			drawLine(pos, pos + half_cone_1 * combatViewDistance, VEC4(1, 0, 0, 1));
			drawLine(pos, pos + half_cone_2 * combatViewDistance, VEC4(1, 0, 0, 1));
			drawLine(pos + half_cone_1 * combatViewDistance, pos + half_cone_2 * combatViewDistance, VEC4(1, 0, 0, 1));
		}
	}
}

void CAISushi::onDamageInfoMsg(const TMsgDamageToEnemy& msg) {
	if (isBlocking) {
		dbg("Damage blocked %\n");
	}
	else {
		h_sender = msg.h_sender;
		CEntity* e_source = (CEntity *)msg.h_bullet;
		TCompTransform* trans_source = e_source->get<TCompTransform>();
		damageSource = trans_source->getPosition();
		life -= msg.intensityDamage;
		if (life < 0)
			life = 0;
		ChangeState("IMPACT");
	}
}

void CAISushi::onGravity(const TMsgGravity& msg) {
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
		attractionForce = msg.attractionForce;
		distanceToBattery = msg.distance;

		ChangeState("BATTERYEFFECT");
	}if (beingAttracted && h_bullet == msg.h_bullet) {

		//obtener posicion
		beingAttracted = true;
		CEntity* e_bullet = (CEntity *)h_bullet;
		TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
		battery_position = bullet_trans->getPosition();

		ChangeState("BATTERYEFFECT");
	}
}

void CAISushi::onCollision(const TMsgOnContact& msg) {
	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	if (source_of_impact) {
		TCompTags* c_tag = source_of_impact->get<TCompTags>();
		if (c_tag) {
			std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
			//If I collide with the player while I'm charging, I send TMsgDamageToPlayer
			if (strcmp("player", tag.c_str()) == 0) {
				if (getState() == "CHARGE") {
					Send_DamageMessage<TMsgDamageToPlayer>(source_of_impact, chargeDamage);
					ChangeState("IDLEWAR");
					return;
				}
				if (getState() == "JUMPCHARGE") {
					Send_DamageMessage<TMsgDamageToPlayer>(source_of_impact, jumpChargeDamage);
					ChangeState("IDLEWAR");
					return;
				}
				//If I collide with something other than the player, but I'm charging, I stop charging
			}
			else if (getState() == "CHARGE" || getState() == "JUMPCHARGE") {
				ChangeState("IDLEWAR");
			}
		}
	}
}

void CAISushi::registerMsgs() {
	DECL_MSG(CAISushi, TMsgDamageToEnemy, onDamageInfoMsg);
	DECL_MSG(CAISushi, TMsgOnContact, onCollision);
	DECL_MSG(CAISushi, TMsgGravity, onGravity);
}