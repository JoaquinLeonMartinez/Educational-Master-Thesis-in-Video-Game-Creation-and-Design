#include "mcv_platform.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/john/comp_bullet.h"
#include "components/common/comp_tags.h"
#include "time.h"
#include "stdlib.h"
#include "skeleton/comp_bone_tracker_golem.h"
#include "modules/module_physics.h"
#include "components/animation/comp_golem_animation.h"
#include "components/controllers/character/comp_character_controller.h"
#include "bt_golem.h"


using namespace physx;
std::mt19937 bt_gol(1129);
std::uniform_int_distribution<int> bt_dist_gol(0, 100);

std::mt19937 bt_gol_throw(1329);
std::uniform_int_distribution<int> bt_range_products(0, 8);

DECL_OBJ_MANAGER("bt_golem", CBTGolem);
void CBTGolem::create(string s)//crear el arbol
{
	name = s; //createRoot y addChild debe hacerse en orden, sino peta -- nunca definir un huerfano directamente 
	// padre - hijo - tipo - condition - action
	createRoot("GOLEM", PRIORITY, NULL, NULL);


	//addChild("GOLEM", "ON_DEATH", ACTION, (btcondition)&CBTGolem::conditionDeath, (btaction)&CBTGolem::actionDeath);
	//addChild("GOLEM", "ON_IMPACT", ACTION, (btcondition)&CBTGolem::conditionImpactReceived, (btaction)&CBTGolem::actionImpactReceived);
	//addChild("GOLEM", "ON_GRAVITY", ACTION, (btcondition)&CBTGolem::conditionGravityReceived, (btaction)&CBTGolem::actionGravityReceived);
	

	addChild("GOLEM", "ATTACKCINEMATIC", ACTION, (btcondition)&CBTGolem::conditionAttackCinematic, (btaction)&CBTGolem::actionMeleeCinematic);
	//inCinematic
	addChild("GOLEM", "IN_CINEMATIC", ACTION, (btcondition)&CBTGolem::conditionCinematic, (btaction)&CBTGolem::nothing);

	addChild("GOLEM", "INSIGHT", SEQUENCE, (btcondition)&CBTGolem::conditionView, NULL);//(btcondition)&CBTGolem::conditionEscape  // (btaction)&CBTGolem::actionIdle
	addChild("GOLEM", "IDLE", ACTION, NULL, (btaction)&CBTGolem::actionIdle);
	
	addChild("INSIGHT", "ALIGN", ACTION, NULL, (btaction)&CBTGolem::actionAlign);
	addChild("INSIGHT", "ATTACK", PRIORITY, NULL, NULL);

	addChild("ATTACK", "THROW", PRIORITY, (btcondition)&CBTGolem::conditionDistanceThrow, NULL);
	addChild("ATTACK", "MELEE", PRIORITY, NULL, NULL);//(btcondition)&CBTGolem::conditionDistanceMelee

	addChild("THROW", "CHARGINGTHROW", ACTION, (btcondition)&CBTGolem::conditionTimerThrow, (btaction)&CBTGolem::actionChargingThrow); //CARGANDO

	//addChild("THROW", "THROWINGCUPCAKE", ACTION, (btcondition)&CBTGolem::conditionRandomThrowCupcake, (btaction)&CBTGolem::actionThrowCupcake);
	addChild("THROW", "THROWINGPROYECTILE", ACTION, (btcondition)&CBTGolem::conditionRandomThrowParabolic, (btaction)&CBTGolem::actionThrow);
	addChild("THROW", "THROWINGNFORTUNECOOKIESIMPLE", ACTION, (btcondition)&CBTGolem::conditionRandomThrowSimpleCookie, (btaction)&CBTGolem::actionThrowCookieSimple);
	addChild("THROW", "THROWINGNFORTUNECOOKIETRIPLE", ACTION, NULL, (btaction)&CBTGolem::actionThrowCookieSpread);

	addChild("MELEE", "CHARGINGMELEE", ACTION, (btcondition)&CBTGolem::conditionTimerMelee, (btaction)&CBTGolem::actionChargingMelee);
	addChild("MELEE", "MELEEATACK", ACTION, NULL, (btaction)&CBTGolem::actionMelee);



	if (!h_player.isValid()) {
		h_player = getEntityByName("Player");
	}

    _painAudio = EngineAudio.playEvent("event:/Enemies/Golem/Golem_Pain");
    _painAudio.stop();
}


int CBTGolem::actionIdle()
{
	//dbg("%s: handling idle\n", name.c_str());
	//cuando animacion este poner return stay
	return LEAVE;	
}


int CBTGolem::nothing()
{
	//dbg("%s: handling idle\n", name.c_str());
	//cuando animacion este poner return stay
	return LEAVE;
}

int CBTGolem::actionAlign()
{
	//dbg("%s: handling action align\n", name.c_str());
	
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);

	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) <= cone_vision) {

		return LEAVE;
	}
	else {
		if (c_trans->isInLeft(player_position->getPosition())) {
			c_trans->setAngles(yaw + dt * twistSpeed, pith);
		}
		else {
			c_trans->setAngles(yaw - dt * twistSpeed, pith);
		}
		return STAY;
	}
}
/*
int attackInCinematic() {
	
		TCompTransform* c_trans = get<TCompTransform>();
		CEntity* e_player = (CEntity *)h_player;
		//TCompTransform* p_trans = e_player->get<TCompTransform>();
		//dbg("%s: handling action melee\n", name.c_str());
		TCompCollider* comp_collider = get<TCompCollider>();
		//Create a collider sphere where we want to detect collision
		PxSphereGeometry geometry(4.0); //NO HARDCODEADO

		Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeRange);
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
					if (player == h_player || attackCinematic) { //TODO: 
						//Send damage mesage
						CEntity* entityContact = hitCollider.getOwner();
						Send_DamageMessage<TMsgDamage>(entityContact, meleeDamage);
					}
				}
			}
		}

		attackCinematic = false;

		return LEAVE;

}*/

int CBTGolem::actionMeleeCinematic()
{

	TCompTransform* c_trans = get<TCompTransform>();
	//Create a collider sphere where we want to detect collision
	PxSphereGeometry geometry(meleeRange*3);
	Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeRange*3);
	PxF32 attackHeight = c_trans->getPosition().y;
	damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;
	PxVec3 pos = VEC3_TO_PXVEC3(damageOrigin);
	PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());

	const PxU32 bufferSize = 256;
	PxOverlapHit hitBuffer[bufferSize];
	PxOverlapBuffer buf(hitBuffer, bufferSize);
	PxTransform shapePose = PxTransform(pos, ori);
	PxQueryFilterData filter_data = PxQueryFilterData();
	filter_data.data.word0 = EnginePhysics.Enemy | EnginePhysics.Puddle | EnginePhysics.DestroyableWall;
	bool res = EnginePhysics.gScene->overlap(geometry, shapePose, buf, filter_data);
	if (res) {
		for (PxU32 i = 0; i < buf.nbTouches; i++) {
			CHandle h_comp_physics;
			h_comp_physics.fromVoidPtr(buf.getAnyHit(i).actor->userData);
			CEntity* entityContact = h_comp_physics.getOwner();
			if (entityContact) {
				TMsgDamage msg;
				// Who sent this bullet
				msg.h_sender = CHandle(this).getOwner();
				msg.h_bullet = CHandle(this).getOwner();
				msg.position = c_trans->getPosition() + VEC3::Up;
				msg.senderType = PLAYER;
				msg.intensityDamage = meleeDamage;
				msg.impactForce = impactForceCinematic;
				msg.damageType = MELEE;
				msg.targetType = ENEMIES;
				entityContact->sendMsg(msg);

			}
		}
	}

	//ANIMATION-----------------------
	TCompGolemAnimator* golemAnimator = get<TCompGolemAnimator>();
	golemAnimator->playAnimation(TCompGolemAnimator::POUND_ATTACK, 1.0f);
	//END ANIMATION------------------------

	attackCinematic = false;
	/*
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	//TCompTransform* p_trans = e_player->get<TCompTransform>();
	//dbg("%s: handling action melee\n", name.c_str());
	TCompCollider* comp_collider = get<TCompCollider>();
	//Create a collider sphere where we want to detect collision
	PxSphereGeometry geometry(4.0); //NO HARDCODEADO

	Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeRange);
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
				if (player == h_player || attackCinematic) { //TODO: 
					//Send damage mesage
					CEntity* entityContact = hitCollider.getOwner();
					Send_DamageMessage<TMsgDamage>(entityContact, meleeDamage);
				}
			}
		}
	}

	//ANIMATION-----------------------
	TCompGolemAnimator* golemAnimator = get<TCompGolemAnimator>();
	golemAnimator->playAnimation(TCompGolemAnimator::POUND_ATTACK, 1.0f);
	//END ANIMATION------------------------
	timerMelee = meleeFrequency;
	*/
    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Golem/Golem_Appear");
    audio.set3DAttributes(*c_trans);
    return LEAVE;

}


int CBTGolem::actionMelee()
{
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	//TCompTransform* p_trans = e_player->get<TCompTransform>();
	//dbg("%s: handling action melee\n", name.c_str());
	TCompCollider* comp_collider = get<TCompCollider>();
	//Create a collider sphere where we want to detect collision
	PxSphereGeometry geometry(4.0); //NO HARDCODEADO

	Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeRange);
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
				if (player == h_player || attackCinematic) { //TODO: 
					//Send damage mesage
					CEntity* entityContact = hitCollider.getOwner();
					Send_DamageMessage<TMsgDamage>(entityContact, meleeDamage);
				}
			}
		}
	}

	//ANIMATION-----------------------
	TCompGolemAnimator* golemAnimator = get<TCompGolemAnimator>();
	//golemAnimator->playAnimation(TCompGolemAnimator::POUND_ATTACK, 1.0f);
	golemAnimator->playAnimation(TCompGolemAnimator::POUND, 1.0f);
	//TCompGolemAnimator::POUND
	//END ANIMATION------------------------
	timerMelee = meleeFrequency;
    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Golem/Golem_Pound");
    audio.set3DAttributes(*c_trans);

	return LEAVE;
	
}



int CBTGolem::actionThrowCupcake()
{
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	
	//ANIMATION-----------------------
	TCompGolemAnimator* golemAnimator = get<TCompGolemAnimator>();
	golemAnimator->playAnimation(TCompGolemAnimator::THROW, 1.0f);
	//END ANIMATION------------------------
    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Golem/Golem_Attack");
    audio.set3DAttributes(*c_trans);

	delay = delayCupcake;
	timerGrenade = throwFrequecy;

	throwActive = true;
	throwType = 1;
	
    return LEAVE;

}


int CBTGolem::actionThrowCookieSpread() {
	_burstTimer -= dt;
    if (!throwAudio) {
        TCompTransform* c_trans = get<TCompTransform>();
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Golem/Golem_Attack");
        audio.set3DAttributes(*c_trans);
        throwAudio = true;
    }
	if (_shotsFired < numberOfCookiesTriple) {
		TCompTransform* c_trans = get<TCompTransform>();
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		c_trans->rotateTowards(p_trans->getPosition(), twistSpeed, dt);
		if (_burstTimer <= 0) {
			dbg("Firing BURST_SHOT\n");

			//ANIMATION-----------------------
			TCompGolemAnimator* golemAnimator = get<TCompGolemAnimator>();
			golemAnimator->playAnimation(TCompGolemAnimator::THROW, 2.0f);
			//END ANIMATION------------------------

			spreadShot();
			_burstTimer = _burstDelay;
			_shotsFired++;
			return STAY;
		}
		else {
			return STAY;
		}
	}
	else {        
        throwAudio = false;
        _shotsFired = 0;
		timerGrenade = throwFrequecy;
		return LEAVE;
	}

}


int CBTGolem::actionThrowCookieSimple() {

	_burstTimer -= dt;

	if (_shotsFired < numberOfCookiesSimple) {
		TCompTransform* c_trans = get<TCompTransform>();
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		c_trans->rotateTowards(p_trans->getPosition(), twistSpeed, dt);
		if (_burstTimer <= 0) {
			dbg("Firing BURST_SHOT\n");

		//ANIMATION-----------------------
		TCompGolemAnimator* golemAnimator = get<TCompGolemAnimator>();
		golemAnimator->playAnimation(TCompGolemAnimator::THROW, 2.0f);
		//END ANIMATION------------------------

			singleShot();
			_burstTimer = _burstDelay;
			_shotsFired++;
			return STAY;
		}
		else {
			return STAY;
		}
	}
	else {
		_shotsFired = 0;
	timerGrenade = throwFrequecy;
		return LEAVE;
	}

	
}

int CBTGolem::actionThrow()
{
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();


	//ANIMATION-----------------------
	TCompGolemAnimator* golemAnimator = get<TCompGolemAnimator>();
	golemAnimator->playAnimation(TCompGolemAnimator::THROW, 1.0f);
	//END ANIMATION------------------------

  AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Golem/Golem_Attack");
  audio.set3DAttributes(*c_trans);
  delay = projectileDelay;
	throwActive = true;
	throwType = 2;

	timerGrenade = throwFrequecy;
	return LEAVE;

}


int CBTGolem::actionGravityReceived() {
	return LEAVE;
}

int CBTGolem::actionDeath() {
	GameController.addEnemiesKilled(EntityType::GOLEM);
	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();
    TCompTransform* c_trans = get<TCompTransform>();
    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Golem/Golem_Death");
    audio.set3DAttributes(*c_trans);
    return LEAVE;
}

int CBTGolem::actionImpactReceived() {
	life = life - intensityDamage;
	//dbg("El golem recibe el impacto, life = %f \n", life);
	return LEAVE;
}

int CBTGolem::actionChargingThrow() {
	return LEAVE;
}

int CBTGolem::actionChargingMelee() {
	return LEAVE;
}

bool CBTGolem::conditionView()
{
	
	CEntity* e_player = GameController.getPlayerHandle();
	if (e_player == nullptr) {
		return false;
	}
	TCompCharacterController* character = e_player->get<TCompCharacterController>();
	if (character->life <= 0) {
		return false;
	}
	/*if (player_dead) {
		return false; 
	}*/

	if (scriptedGolem && !golemCinematic) {
		return false;
	}
	//en realidad, al estar scripteado el isView deja de ser necesario
	return isView(); /*|| golemCinematic; */ //si es un golem activado por cinematica siempre te ve hasta que se desactive()
}

bool CBTGolem::conditionCinematic()
{
	return inCinematic;
}

bool CBTGolem::conditionAttackCinematic()
{
	return attackCinematic;
}

bool CBTGolem::isView() {
	
	if (!h_player.isValid()) {
		h_player = GameController.getPlayerHandle();
	}
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distance = Vector3::Distance(c_trans->getPosition(), player_position->getPosition());
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));

	bool sighted = ((abs(angle) <= half_cone) && (distance <= viewDistance)) || distance <= hearing_radius;
	TCompCharacterController* character = e_player->get<TCompCharacterController>();
	if (character->life <= 0) {
		return false;
	}


	return sighted && checkHeight();
}

bool CBTGolem::conditionDistanceThrow()
{
	
	//dbg("%s: handling condition distance\n", name.c_str());
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distance = VEC3::Distance(player_position->getPosition(), c_trans->getPosition());
	//dbg("Distancia %f\n",distance);
	
	if (distance > epsilon) {
		//dbg("Dispara cupcake\n");
		
		return true; 
	}
	else {
		//dbg("No dispara\n");
		return false;
	}
}

bool CBTGolem::conditionDistanceMelee()
{
	//dbg("%s: handling condition distance\n", name.c_str());
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distance = VEC3::Distance(player_position->getPosition(), c_trans->getPosition());
	//dbg("Distancia %f\n", distance);
	if (distance <= epsilon) {
		//dbg("Pega melee\n");
		return true;
	}
	else {
		//dbg("No Pega melee\n");
		return false;
	}
}

bool CBTGolem::conditionTimerThrow() {
	if (timerGrenade > 0) {
		timerGrenade -= dt;
		return true;
	}
	randomNumber = bt_dist_gol(bt_gol);
	return false;
}

bool CBTGolem::conditionRandomThrowCupcake() {
	
	if(!noThrowCupcake) {
		randomNumber = bt_dist_gol(bt_gol);
		if (randomNumber < throwCupcakeProbability && _currentEnemies.size() < _spawnMaxNumber -1) {
			return true; //throw cupcake
		}
	}
	return false;//check others
}

bool CBTGolem::conditionRandomThrowParabolic() {
	if (randomNumber < throwParabolicProjectileProb) { //&& _currentEnemies.size() < _spawnMaxNumber
		return true; //throw parabolic proyectile
	}
	return false;//check others
}

bool CBTGolem::conditionRandomThrowSimpleCookie() {
	if (randomNumber < throwSimpleFortuneCookieProb) { //&& _currentEnemies.size() < _spawnMaxNumber
		return true; //throw Cookie
	}
	return false;//throw Triple Cookie
}

bool CBTGolem::conditionTimerMelee() {
	if (timerMelee > 0) {
		timerMelee -= dt;
		return true;

	}
	return false;
}

bool CBTGolem::conditionDeath() {
	return life <= 0.f;
}

bool CBTGolem::conditionImpactReceived() {

	if (life >0 && damageaded) {
		damageaded = false;
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* player_position = e_player->get<TCompTransform>();
		TCompTransform* c_trans = get<TCompTransform>();

		float yaw, pith;
		c_trans->getAngles(&yaw, &pith);

		float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
		if ( abs(angle) > cone_vision) {
			return true;

		}
	}

	return false;
}


bool CBTGolem::conditionGravityReceived() {
	if (battery_time > 0) {
		battery_time -= dt;
		//dbg("battery time = %f \n", battery_time);
		return true;
	
	}
	return false;
}



template <typename T>
void CBTGolem::Send_DamageMessage(CEntity* entity, float dmg) {
	//Send damage mesage
	T msg;
	// Who sent this bullet
	msg.h_sender = h_sender;
	// The bullet information 
	msg.h_bullet = CHandle(this).getOwner();
	msg.intensityDamage = dmg;
	msg.senderType = ENEMIES;
	msg.targetType = PLAYER;
	msg.impactForce = impactForceAttack;
	entity->sendMsg(msg);

	//dbg("ataque melee enviado al player \n");
}

void CBTGolem::load(const json& j, TEntityParseContext& ctx) {
	

	half_cone = j.value("halfCone", half_cone);
	length_cone = j.value("lengthCone", length_cone);
	hearing_radius = j.value("hearingRadius", hearing_radius);
	life = j.value("life", life);
	damageArea = j.value("damageArea", damageArea);

	this->create("Golem_bt");

}


void CBTGolem::debugInMenu() {
	ImGui::Checkbox("_pausedBT", &_pausedBT);
}

void CBTGolem::renderDebug() {
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



void CBTGolem::onCollision(const TMsgOnContact& msg) {
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
		else if (strcmp("grenade", tag.c_str()) == 0) {
			//ChangeState("IMPACT");
			//dbg("Daño granada");
			
		}
		else if (strcmp("bullet", tag.c_str()) == 0) {
			//ChangeState("IMPACT");
			//dbg("Daño bala");
		}
	}
}


void CBTGolem::onGravity(const TMsgGravity& msg) {
	//	Paralyze golem during a timer

	h_sender = msg.h_sender;
	h_bullet = msg.h_bullet;
	//obtener posicion

	CEntity* e_bullet = (CEntity *)h_bullet;
	TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
	//battery_position = bullet_trans->getPosition();
	battery_time = msg.time_effect;
}
/*
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
*/

void CBTGolem::onDamageInfoMsg(const TMsgDamage& msg) {
	/*h_sender = msg.h_sender;
	CEntity* e_source = (CEntity *)msg.h_bullet;
	TCompTransform* trans_source = e_source->get<TCompTransform>();
	life -= msg.intensityDamage;
	if (life < 0)
		life = 0;*/
	h_sender = msg.h_sender;
	CEntity* e_source = (CEntity *)msg.h_bullet;
	TCompTransform* trans_source = e_source->get<TCompTransform>();
	intensityDamage = msg.intensityDamage;
    if (!_painAudio.isPlaying()) {
        _painAudio.restart();
    }
	damageaded = true;
}


void CBTGolem::registerMsgs() {
	//DECL_MSG(CBTGolem, TMsgDamage, onDamageInfoMsg);
	DECL_MSG(CBTGolem, TMsgOnContact, onCollision);
	DECL_MSG(CBTGolem, TMsgGravity, onGravity);
	DECL_MSG(CBTGolem, TMsgBTPaused, onMsgBTPaused);
	DECL_MSG(CBTGolem, TMsgSpawnerCheckout, onCheckout);
	DECL_MSG(CBTGolem, TMsgSpawnerFather, onBornChild);
	DECL_MSG(CBTGolem, TMsgActiveGolem, activeGolem);
	DECL_MSG(CBTGolem, TMsgOnCinematic, onCinematic);
}


void CBTGolem::onCinematic(const TMsgOnCinematic & msg) {
	inCinematic = msg.cinematic;
}

void CBTGolem::activeGolem(const TMsgActiveGolem & msg) {
	golemCinematic = msg.active;
	attackCinematic = true;//para romper el muro

	//attackCinematic = true;
}

void CBTGolem::onCheckout(const TMsgSpawnerCheckout & msg) {
	for (auto it = _currentEnemies.begin(); it != _currentEnemies.end(); ++it) {
		if (*it == msg.enemyHandle) {
			_currentEnemies.erase(it);
			return;
		}
	}
}

void CBTGolem::onBornChild(const TMsgSpawnerFather & msg) {
	_currentEnemies.push_back(msg.son);
}


void CBTGolem::singleShot() {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompCollider* p_col = e_player->get<TCompCollider>();
	TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_cc = get<TCompCollider>();
	//Bullet origin
	TCompBoneTrackerGolem* boneTracker = get<TCompBoneTrackerGolem>();
	VEC3 firingPosition = boneTracker->getPosition();
	firingPosition.y += c_cc->controller->getHeight();
	firingPosition += c_trans->getFront() * 0.5f;

	TEntityParseContext ctx;
	ctx.root_transform.setPosition(firingPosition);
	ctx.root_transform.setRotation(c_trans->getRotation());
	

	productToThrow = bt_range_products(bt_gol_throw);
	parseScene(objectToThrow.at(productToThrow), ctx);

	//Bullet direction
	VEC3 targetDir = p_trans->getPosition() - boneTracker->getPosition();
	targetDir.Normalize();
	VEC3 _targetPosition = p_trans->getPosition() + targetDir * _playerOffset;
	VEC3 _targetDirection = _targetPosition - boneTracker->getPosition();
	_targetDirection.Normalize();

	//Message to the player
	TMsgDamage msgDamage;
	msgDamage.bullet_front = _targetDirection;
	msgDamage.senderType = EntityType::GOLEM;
	msgDamage.targetType = EntityType::PLAYER;
	msgDamage.intensityDamage = _bulletDamage;
	msgDamage.impactForce = _bulletForce;

	//Message to the bullet
	TMsgAssignBulletOwner msg;
	msg.h_owner = CHandle(this).getOwner();
	msg.h_target = h_player;
	msg.source = c_trans->getPosition();
	msg.front = _targetDirection;
	msg.messageToTarget = msgDamage;
	ctx.entities_loaded[0].sendMsg(msg);
}

void CBTGolem::spreadShot() {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompCollider* p_col = e_player->get<TCompCollider>();
	TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_cc = get<TCompCollider>();

	//STRAIGHT BULLET
	//Bullet origin
	TCompBoneTrackerGolem* boneTracker = get<TCompBoneTrackerGolem>();
	VEC3 firingPosition = boneTracker->getPosition();
	firingPosition.y += c_cc->controller->getHeight();
	firingPosition += c_trans->getFront() * 0.5f;

	TEntityParseContext ctx;
	ctx.root_transform.setPosition(firingPosition);
	ctx.root_transform.setRotation(c_trans->getRotation());

	productToThrow = bt_range_products(bt_gol_throw);
	parseScene(objectToThrow.at(productToThrow), ctx); //1

	//objectToThrow

	//Bullet direction
	VEC3 targetDir = p_trans->getPosition() - boneTracker->getPosition();
	targetDir.Normalize();
	VEC3 _targetPosition = p_trans->getPosition() + targetDir * _playerOffset;
	VEC3 _targetDirection = _targetPosition - boneTracker->getPosition();
	_targetDirection.Normalize();

	//Message to the player
	TMsgDamage msgDamage;
	msgDamage.bullet_front = _targetDirection;
	msgDamage.senderType = EntityType::GOLEM;
	msgDamage.targetType = EntityType::PLAYER;
	msgDamage.intensityDamage = _bulletDamage;
	msgDamage.impactForce = _bulletForce;

	//Message to the bullet
	TMsgAssignBulletOwner msg;
	msg.h_owner = CHandle(this).getOwner();
	msg.h_target = h_player;
	msg.source = c_trans->getPosition();
	msg.front = _targetDirection;
	msg.messageToTarget = msgDamage;
	ctx.entities_loaded[0].sendMsg(msg);
	//END STRAIGHT BULLET

	//15DEG BULLET
	VEC3 rotatedDirection = VEC3().Zero;
	MAT44 rotMat = MAT44::CreateRotationY(deg2rad(10.f));
	VEC3().Transform(_targetDirection, rotMat, rotatedDirection);

	TEntityParseContext ctx2;
	ctx2.root_transform.setPosition(firingPosition);
	ctx2.root_transform.setRotation(c_trans->getRotation());

	productToThrow = bt_range_products(bt_gol_throw);
	parseScene(objectToThrow.at(productToThrow), ctx2); // 2

	msgDamage.bullet_front = rotatedDirection;
	msg.front = rotatedDirection;
	ctx2.entities_loaded[0].sendMsg(msg);
	//END 15DEG BULLET

	//-15DEG BULLET
	rotatedDirection = VEC3().Zero;
	rotMat = MAT44::CreateRotationY(deg2rad(-10.f));
	VEC3().Transform(_targetDirection, rotMat, rotatedDirection);

	TEntityParseContext ctx3;
	ctx3.root_transform.setPosition(firingPosition);
	ctx3.root_transform.setRotation(c_trans->getRotation()); 

	productToThrow = bt_range_products(bt_gol_throw);
	parseScene(objectToThrow.at(productToThrow), ctx3); // 3
	msgDamage.bullet_front = rotatedDirection;
	msg.front = rotatedDirection;
	ctx3.entities_loaded[0].sendMsg(msg);
	//END -15DEG BULLET
}

void CBTGolem::updateBT() {

	if (firstExec) {
		firstExec = false;

		//---------------------------- 

		CHandle h(this);
		CHandle c_e = h.getOwner();
		CEntity* c_entity = (CEntity*)c_e;
		//obtener el nombre de la entidad y ponerselo a comp_bone_tracker_golem
		TCompBoneTrackerGolem* boneTracker = get<TCompBoneTrackerGolem>();
		
		TCompName* myName = get<TCompName>();
		boneTracker->setParentName(myName->getName());
		
		//---------------------------------

	}

	if (throwActive) {

		if (delay <= 0) {
			TCompTransform* c_trans = get<TCompTransform>();
			TCompBoneTrackerGolem* boneTracker = get<TCompBoneTrackerGolem>();
			
			TCompSkeleton *h_skeleton = get<TCompSkeleton>();
			
			VEC3 posBrazoLanzamiento = h_skeleton->getBonePositionByName("Bone011_izq");
			TEntityParseContext ctx;
			ctx.root_transform.setPosition(posBrazoLanzamiento);
			


			if (throwType == 1) { //cupcake
				
				parseScene("data/prefabs/bullets/grenade_golem.json", ctx);
				TMsgAssignBulletOwner msg;
				msg.h_owner = CHandle(this).getOwner();
				msg.source = posBrazoLanzamiento;
				//msg.source = c_trans->getPosition();
				msg.front = c_trans->getFront();
				ctx.entities_loaded[0].sendMsg(msg);
				//delay = delayCupcake;
				
			}
			else { //parabolic

				parseScene("data/prefabs/bullets/turret_bullet.json", ctx);
				//delay = projectileDelay;
				TMsgAssignBulletOwner msg;
				msg.h_owner = CHandle(this).getOwner();
				msg.source = posBrazoLanzamiento;
				msg.front = c_trans->getFront();
				ctx.entities_loaded[0].sendMsg(msg);
			}
			throwActive = false;
		}
		else {
			delay -= dt;
		}
	}


}


bool CBTGolem::checkHeight() {
	bool res = false;
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	float playerHeight = player_position->getPosition().y;
	TCompTransform* c_trans = get<TCompTransform>();
	float enemyHeight = c_trans->getPosition().y;

	if (height_range > abs(playerHeight - enemyHeight)) {
		res = true;;
	}

	return res;
}

void CBTGolem::setViewDistance(float distance) {
	viewDistance = distance;
}

void CBTGolem::setHeightRange(float height) {
	this->height_range = height;
}

void CBTGolem::setNotThrowCupcake(bool value) {
	this->noThrowCupcake = value;
}