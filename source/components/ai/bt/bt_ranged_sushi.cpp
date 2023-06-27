#include "mcv_platform.h"
#include "engine.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/common/comp_name.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/john/comp_bullet.h"
#include "components/common/comp_tags.h"
#include "modules/module_physics.h"
#include "modules/game/module_fluid_decal_generator.h"
#include "components/animation/comp_sushi_animation.h"
#include "components/ai/others/comp_blackboard.h"
#include "bt_ranged_sushi.h"
#include "components/objects/comp_enemies_in_butcher.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/ai/others/self_destroy.h"
#include "components/vfx/comp_death_billboard.h"

#include "random"

using namespace physx;
std::mt19937 bt_mt_rs(std::random_device{}());
std::uniform_int_distribution<int> bt_dist_rs(0, 100);

DECL_OBJ_MANAGER("bt_ranged_sushi", CBTRangedSushi);
void CBTRangedSushi::create(string s)//crear el arbol
{
	name = s; //createRoot y addChild debe hacerse en orden, sino peta -- nunca definir un huerfano directamente 
		  // padre - hijo - tipo - condition - action
	createRoot("SUSHI", PRIORITY, NULL, NULL);
	addChild("SUSHI", "ON_DEATH", ACTION, (btcondition)&CBTRangedSushi::conditionDeath, (btaction)&CBTRangedSushi::actionDeath);
	addChild("SUSHI", "ON_GRAVITY", ACTION, (btcondition)&CBTRangedSushi::conditionGravityReceived, (btaction)&CBTRangedSushi::actionGravityReceived);
	addChild("SUSHI", "ON_FEAR", ACTION, (btcondition)&CBTRangedSushi::conditionFear, (btaction)&CBTRangedSushi::actionFear);
	addChild("SUSHI", "ON_IMPACT", ACTION, (btcondition)&CBTRangedSushi::conditionImpactReceived, (btaction)&CBTRangedSushi::actionImpactReceived);
	addChild("SUSHI", "ON_AIR", ACTION, (btcondition)&CBTRangedSushi::conditionOnAir, (btaction)&CBTRangedSushi::actionOnAir);
  addChild("SUSHI", "DEATH", ACTION, (btcondition)& CBTRangedSushi::conditionDeathAnimation, (btaction)& CBTRangedSushi::actionDeathStay);

	addChild("SUSHI", "VIEW", PRIORITY, (btcondition)&CBTRangedSushi::conditionPlayerInView, NULL);
	//addChild("VIEW", "SALUTE", ACTION, (btcondition)&CBTRangedSushi::conditionSalute, (btaction)&CBTRangedSushi::actionSalute);
	addChild("VIEW", "COMBAT_HOLDER", PRIORITY, NULL, NULL);
	//SE HA QUITADO EL ESTADO DE IDLE
	//addChild("SUSHI", "IDLE", ACTION, NULL, (btaction)&CBTRangedSushi::actionIdle);
	//////NO PATRULLEROS COMENTAR ESTAS TRES LINEAS
	addChild("SUSHI", "PATROL", SEQUENCE, NULL, NULL);
	addChild("PATROL", "NEXT_WAYPOINT", ACTION, NULL, (btaction)&CBTRangedSushi::actionNextWaypoint);
	addChild("PATROL", "SEEK_WAYPOINT", ACTION, NULL, (btaction)&CBTRangedSushi::actionSeekWaypoint);
	//////
	addChild("COMBAT_HOLDER", "DECOY", ACTION, (btcondition)&CBTRangedSushi::conditionDecoy, (btaction)&CBTRangedSushi::actionDecoy);
	addChild("COMBAT_HOLDER", "CHASE", ACTION, (btcondition)&CBTRangedSushi::conditionChase, (btaction)&CBTRangedSushi::actionChase);

	addChild("COMBAT_HOLDER", "LEAP_HOLDER", PRIORITY, (btcondition)&CBTRangedSushi::conditionLeap, NULL);
	addChild("LEAP_HOLDER", "BOUNCE", ACTION, (btcondition)&CBTRangedSushi::conditionBounce, (btaction)&CBTRangedSushi::actionBounce);
	addChild("LEAP_HOLDER", "LEAP", ACTION, NULL, (btaction)&CBTRangedSushi::actionLeap);

	addChild("COMBAT_HOLDER", "SHOOT_HOLDER", RANDOM, (btcondition)&CBTRangedSushi::conditionShoot, NULL);
	addChild("SHOOT_HOLDER", "SINGLE_SHOT", ACTION, NULL, (btaction)&CBTRangedSushi::actionSingleShot);
	addChild("SHOOT_HOLDER", "SPREAD_SHOT", ACTION, NULL, (btaction)&CBTRangedSushi::actionSpreadShot);
	addChild("SHOOT_HOLDER", "BURST_SHOT", ACTION, NULL, (btaction)&CBTRangedSushi::actionBurstShot);

	addChild("COMBAT_HOLDER", "RETREAT", ACTION, (btcondition)&CBTRangedSushi::conditionRetreat, (btaction)&CBTRangedSushi::actionRetreat);

	/*addChild("COMBAT_HOLDER", "ORBIT_HOLDER", RANDOM, (btcondition)&CBTRangedSushi::conditionOrbit, NULL);
	addChild("ORBIT_HOLDER", "ORBIT_RIGHT", ACTION, NULL, (btaction)&CBTRangedSushi::actionOrbitRight);
	addChild("ORBIT_HOLDER", "ORBIT_LEFT", ACTION, NULL, (btaction)&CBTRangedSushi::actionOrbitLeft);*/

	addChild("COMBAT_HOLDER", "IDLE_COMBAT", ACTION, NULL, (btaction)&CBTRangedSushi::actionIdleCombat);

	if (!h_player.isValid()) {
		h_player = GameController.getPlayerHandle();
	}
	if (_initiallyPaused) {
		setPaused(true);
	}

    _footSteps = EngineAudio.playEvent("event:/Enemies/Sushi/Sushi_Footsteps");
    _footSteps.setPaused(true);
}

bool CBTRangedSushi::conditionDeathAnimation() {
  return life <= 0.f && death_animation_started;
}

int CBTRangedSushi::actionDeathStay() {

  return STAY;
}

void CBTRangedSushi::updateBT() {
	_actionTimer -= dt;
	_shootTimer -= dt;
	_decoyCooldownTimer -= dt;

	if (isGrounded() && _hasBounced)
		_hasBounced = false;
	//_orbitCooldownTimer -= dt;

	reevaluatePathTimer -= dt;

	TCompTransform* c_trans = get<TCompTransform>();
    _footSteps.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
    if (nextNavMeshPoint != VEC3().Zero	&& use_navmesh) { //update path point
		if (Vector3::Distance(nextNavMeshPoint, c_trans->getPosition()) < distanceCheckThreshold) {
			navMeshIndex++;
			if (navMeshIndex < navmeshPath.size()) {
				nextNavMeshPoint = navmeshPath[navMeshIndex];
			}

		}
	}


	//check if is in the blackboard
	if (checkBlackboard()) {
		if (resetSlotTimer >= resetSlotDuration) {
			resetSlotTimer = 0.0f;
			//LIBERAR SLOT
			slotsAvailable = false;
			CEntity* e_player = (CEntity *)h_player;
			TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
			c_bb->forgetPlayer(CHandle(this).getOwner(), SUSHI);
		}
		else {
			resetSlotTimer += dt;
		}
	}

}

//Actions
#pragma region Actions
int CBTRangedSushi::actionIdle() {
	previousState = currentState;
	currentState = States::Idle;
	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::IDLE_LOOP, 1.f);
    _footSteps.setPaused(true);
	return LEAVE;
}

int CBTRangedSushi::actionIdleCombat() {
	previousState = currentState;
	currentState = States::IdleCombat;
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	c_trans->rotateTowards(p_trans->getPosition());
	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::IDLE_LOOP, 1.f);
    _footSteps.setPaused(true);
	return LEAVE;
}

int CBTRangedSushi::actionNextWaypoint() {
	previousState = currentState;
	currentState = States::NextWaypoint;
	if (_curve == nullptr) {
		if (initialExecution) {
			initialExecution = false;
			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 position = c_trans->getPosition();
			//wtp

			positions.push_back(VEC3(position.x + 5.0f, position.y + 0.0f, position.z + 5.0f)); //estos valores quiza deberian ser por parametro tmb
			positions.push_back(VEC3(position.x + 0.0f, position.y + 0.0f, position.z + 0.0f));
			positions.push_back(VEC3(position.x + 5.0f, position.y + 0.0f, position.z - 5.0f));
			positions.push_back(VEC3(position.x - 5.0f, position.y + 0.0f, position.z + 5.0f));
			nextPoint = positions[wtpIndex];
		}

		wtpIndex = (wtpIndex + 1) % positions.size();
		nextPoint = positions[wtpIndex];
	}
	else {
		if (initialExecution) {
			initialExecution = false;
			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 position = c_trans->getPosition();
			//wtp
			_knots = _curve->_knots;
			//nextPoint = _knots[wtpIndex];

		}

		if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}
		ratio += dt * mTravelTime;
		nextPoint = _curve->evaluate(ratio);
	}
	return LEAVE;
}

void CBTRangedSushi::setCurve(const CCurve* curve) {


	this->_curve = curve; // TO TEST
	_knots = _curve->_knots;
	this->pathCurve = curve->getName();
	/*_knots = curve->_knots;

	positions.clear();
	for (int i = 0; i < _knots.size(); i++) {
		positions.push_back(_knots[i]);
	}*/
}

int CBTRangedSushi::actionSeekWaypoint() {
	previousState = currentState;
	currentState = States::SeekWaypoint;
	if (conditionPlayerInView() || inCombat || conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
		return LEAVE;
	}

	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::WALK_LOOP, 1.f);

	TCompTransform* c_trans = get<TCompTransform>();

	//Gravity control
	Vector3 dir = VEC3();
	dir = c_trans->getFront() * patrolSpeed;
	dir *= dt;

	//MOVE
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());


	if (Vector3::Distance(nextPoint, c_trans->getPosition()) < distanceCheckThreshold) {
		//ChangeState("NEXTWPT");
        _footSteps.setPaused(true);
		return LEAVE;
	}
	else {
		//------------------------------- navmesh code

		if (use_navmesh) {
			if (reevaluatePathTimer <= 0) {
				reevaluatePathTimer = reevaluatePathDelay;

				TCompTransform* c_trans = get<TCompTransform>();
				VEC3 position = c_trans->getPosition();
				//wtp 
				generateNavmesh(position, nextPoint, false);

				if (navmeshPath.size() > 0) {
					navMeshIndex = 0;
					nextNavMeshPoint = navmeshPath[navMeshIndex];
				}

			}

			c_trans->rotateTowards(nextNavMeshPoint, rotationSpeed, dt);
		}
		else {
			c_trans->rotateTowards(nextPoint, rotationSpeed, dt);
		}

		//------------------------------- end navmesh code
        _footSteps.setPaused(false);
        return STAY;
	}
}

int CBTRangedSushi::actionSalute() {
	previousState = currentState;
	currentState = States::Salute;
	hasSaluted = true;
	if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
		return LEAVE;
	}
	TCompName* cname = get<TCompName>();
	if (saluteElapsed < saluteDuration) {
		//Salute code
		saluteElapsed += dt;
		return STAY;
	}
	else {
		dbg("%s leaves SALUTE\n", cname->getName());
		//ChangeState("CHASE");
		_actionTimer = _actionDelay;
		return LEAVE;
	}
}

void CBTRangedSushi::shoot(ShotType type) {
	switch (type) {
	case ShotType::Single:
		singleShot();
		break;
	case ShotType::Spread:
		spreadShot();
		break;
	case ShotType::Burst:
		/*should consider adding burst to the random pool, although that would require coroutines*/
		break;
	case ShotType::Random: 
		int dice = bt_dist_rs(bt_mt_rs);
        if (dice > 0 && dice < 50) {
            singleShot();
        }
        else if (dice > 50 && dice < 100){
			spreadShot();
	    }
		break;
	}
}

void CBTRangedSushi::singleShot() {
    CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompCollider* p_col = e_player->get<TCompCollider>();
	TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_cc = get<TCompCollider>();

    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Ranged_SingleThrow");
    audio.set3DAttributes(*c_trans);
	//Bullet origin
	VEC3 firingPosition = c_trans->getPosition();
	firingPosition.y += c_cc->controller->getHeight();
	firingPosition += c_trans->getFront() * 0.5f;

	TEntityParseContext ctx;
	ctx.root_transform.setPosition(firingPosition);
	ctx.root_transform.setRotation(c_trans->getRotation());
	parseScene("data/prefabs/bullets/bullet_sushi.json", ctx);

	//Bullet direction
	VEC3 targetDir = p_trans->getPosition() - c_trans->getPosition();
	targetDir.Normalize();
	VEC3 _targetPosition = p_trans->getPosition() + targetDir * _playerOffset;
	VEC3 _targetDirection = _targetPosition - c_trans->getPosition();
	_targetDirection.Normalize();

	//Message to the player
	TMsgDamage msgDamage;
	msgDamage.bullet_front = _targetDirection;
	msgDamage.senderType = EntityType::SUSHI;
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

void CBTRangedSushi::spreadShot() {
    CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompCollider* p_col = e_player->get<TCompCollider>();
	TCompTransform* c_trans = get<TCompTransform>();
	TCompCollider* c_cc = get<TCompCollider>();
    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Ranged_SpreadThrow");
    audio.set3DAttributes(*c_trans);

	//STRAIGHT BULLET
	//Bullet origin
	VEC3 firingPosition = c_trans->getPosition();
	firingPosition.y += c_cc->controller->getHeight();
	firingPosition += c_trans->getFront() * 0.5f;

	TEntityParseContext ctx;
	ctx.root_transform.setPosition(firingPosition);
	ctx.root_transform.setRotation(c_trans->getRotation());
	parseScene("data/prefabs/bullets/bullet_sushi.json", ctx);

	//Bullet direction
	VEC3 targetDir = p_trans->getPosition() - c_trans->getPosition();
	targetDir.Normalize();
	VEC3 _targetPosition = p_trans->getPosition() + targetDir * _playerOffset;
	VEC3 _targetDirection = _targetPosition - c_trans->getPosition();
	_targetDirection.Normalize();

	//Message to the player
	TMsgDamage msgDamage;
	msgDamage.bullet_front = _targetDirection;
	msgDamage.senderType = EntityType::SUSHI;
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
	parseScene("data/prefabs/bullets/bullet_sushi.json", ctx2);
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
	parseScene("data/prefabs/bullets/bullet_sushi.json", ctx3);
	msgDamage.bullet_front = rotatedDirection;
	msg.front = rotatedDirection;
	ctx3.entities_loaded[0].sendMsg(msg);
	//END -15DEG BULLET
}

int CBTRangedSushi::actionSingleShot() {
	dbg("Firing SINGLE_SHOT\n");

	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(_isLeaping ? TCompSushiAnimator::THROW_AIR : TCompSushiAnimator::THROW_LAND, 1.f);

	singleShot();

	_shootTimer = _shootDelay;
	return LEAVE;
}

int CBTRangedSushi::actionSpreadShot() {
	dbg("Firing SPREAD_SHOT\n");

	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(_isLeaping ? TCompSushiAnimator::THROW_AIR : TCompSushiAnimator::THROW_LAND, 1.f);

	spreadShot();

	_shootTimer = _shootDelay;
	return LEAVE;
}

int CBTRangedSushi::actionBurstShot() {
	_burstTimer -= dt;

	if (_shotsFired < 3) {
		TCompTransform* c_trans = get<TCompTransform>();
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
		if (_burstTimer <= 0) {
			dbg("Firing BURST_SHOT\n");

			TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
			sushiAnimator->playAnimation(_isLeaping ? TCompSushiAnimator::THROW_AIR : TCompSushiAnimator::THROW_LAND, 2.f);
            if (!hasPlayedTripleThrowAudio) {
                AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Ranged_TripleThrow");
                audio.set3DAttributes(*c_trans);
                hasPlayedTripleThrowAudio = true;
            }
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
		_shootTimer = _shootDelay;
		_shotsFired = 0;
        hasPlayedTripleThrowAudio = false;
		return LEAVE;
	}
}

int CBTRangedSushi::actionLeap() {
	previousState = currentState;
	currentState = States::Leap;
	VEC3 jumpForce = getLeapDirection();
	TCompTransform* c_trans = get<TCompTransform>();
    _footSteps.setPaused(true);

	bool enemyInSide = isOtherEnemyInSide();

	if (_hasBounced || conditionGravityReceived() || conditionImpactReceived() || conditionFear() || !isHole(jumpForce) || enemyInSide) {
		_shootTimer = _shootDelay;
		_isLeaping = false;
		_hasFired = false;
		return LEAVE;
	}
	if (!_isLeaping) {
		TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
		sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_START, 1.f);

        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Sushi_Jump_NoVoice");
        audio.set3DAttributes(*c_trans);
        //Start leap
		dbg("Ranged Sushi LEAPS\n");
		_isLeaping = true;
		//VEC3 jumpForce = getLeapDirection(); //VEC3(1.0, 0.0, 0.0);
		jumpForce.y = 1.f;
		jumpForce *= _leapStrength;
		TCompRigidBody* c_rb = get<TCompRigidBody>();
		c_rb->addForce(jumpForce);
		_jumpSource = c_trans->getPosition();
		_actionTimer = _actionDelay;
		return STAY;
	}
	else if (!isGrounded()) {
		TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
		sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_LOOP, 1.f);

		_shootTimer -= dt;
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
		if (_shootTimer <= 0 && !_hasFired) {

			//Set action timer to 0 instead of shooting, that way the BT should enter actionShoot on its own
			//Randomize shot type
			shoot(ShotType::Random);
			_shootTimer = _shootDelay;
			_hasFired = true;
		}
		return STAY;
	}
	_isLeaping = false;
	_hasFired = false;

	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_END, 1.5f);

	//------------------------------------ Blackboard

	CEntity* e_player = (CEntity *)h_player;
	TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
	slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);

	if (slotsAvailable) {
		slotsAvailable = false;
		c_bb = e_player->get<TCompBlackboard>();
		c_bb->forgetPlayer(CHandle(this).getOwner(), SUSHI);
	}


	//------------------------------------

	return LEAVE;
}

VEC3 CBTRangedSushi::getLeapDirection() {
	float tmpdt;
	if (bt_dist_rs(bt_mt_rs) < 50) {
		//Left
		tmpdt = dt;
		directionJump = 1;
	}
	else {
		//Right
		tmpdt = -dt;
		directionJump = 0;
	}
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	Vector3 p_pos = p_trans->getPosition();
	Vector3 e_pos = c_trans->getPosition();

	float s = sin(tmpdt);
	float c = cos(tmpdt);

	// translate point back to origin:
	e_pos.x -= p_pos.x;
	e_pos.z -= p_pos.z;

	// rotate point
	float xnew = e_pos.x * c - e_pos.z * s;
	float ynew = e_pos.x * s + e_pos.z * c;

	// translate point back:
	e_pos.x = xnew + p_pos.x;
	e_pos.z = ynew + p_pos.z;

	VEC3 dir = e_pos - c_trans->getPosition();
	dir.Normalize();
	return dir;
}

/*
TODO: Find a better solution for the bounce direction, this version produces shit results
when the contact is made without horizontal momentum
*/
int CBTRangedSushi::actionBounce() {
	previousState = currentState;
	currentState = States::Bounce;
	if (conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
		_shootTimer = _shootDelay;
		_isLeaping = false;
		_hasFired = false;
		_hasBounced = false;
		return LEAVE;
	}
    _footSteps.setPaused(true);
    if (!_isLeaping) {
		TCompTransform* c_trans = get<TCompTransform>();
		TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
		sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_START, 1.f);
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Sushi_Jump");
        audio.set3DAttributes(*c_trans);

		//Start leap
		dbg("Ranged Sushi BOUNCES\n");
		_isLeaping = true;
		VEC3 aux = c_trans->getPosition();
		VEC3 jumpForce = _jumpSource - aux;
		jumpForce.y = 1.f;
		jumpForce.Normalize();
		jumpForce *= _bounceStrength;
		TCompRigidBody* c_rb = get<TCompRigidBody>();
		c_rb->addForce(jumpForce);
		_actionTimer = _actionDelay;
		return STAY;
	}
	else if (!isGrounded()) {
		TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
		sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_LOOP, 1.f);

		_shootTimer -= dt;
		TCompTransform* c_trans = get<TCompTransform>();
		CEntity* e_player = (CEntity *)h_player;
		TCompTransform* p_trans = e_player->get<TCompTransform>();
		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
		if (_shootTimer <= 0 && !_hasFired) {
			TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
			sushiAnimator->playAnimation(TCompSushiAnimator::THROW_AIR, 1.f);

			//Randomize shot type
			shoot(ShotType::Random);
			_shootTimer = _shootDelay;
			_hasFired = true;
		}
		return STAY;
	}
	_isLeaping = false;
	_hasFired = false;
	_hasBounced = false;

	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_END, 1.f);

	return LEAVE;
}

int CBTRangedSushi::actionChase() {
	previousState = currentState;
	currentState = States::Chase;
	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::WALK_LOOP, 1.5f);

	TCompName* cname = get<TCompName>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//Movement Control
	Vector3 dir = Vector3();
	dir = c_trans->getFront() * chaseSpeed * dt;
	TCompCollider* c_cc = get<TCompCollider>();
	/*if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());*/
		//End Movement Control

		//Rotation Control
		//LookAt_Player(c_trans, player_position);

		//------------------------------- navmesh code

	if (use_navmesh) {
		if (reevaluatePathTimer <= 0) {
			reevaluatePathTimer = reevaluatePathDelay;

			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 position = c_trans->getPosition();
			//wtp 
			generateNavmesh(position, p_trans->getPosition(), false);
		}
		if (navmeshPath.size() > 0 && hayCamino) {
			navMeshIndex = 0;
			nextNavMeshPoint = navmeshPath[navMeshIndex];

			if (c_cc)
				c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
			c_trans->rotateTowards(nextNavMeshPoint, rotationSpeed, dt);
		}
		

	}
	else {
		if (c_cc)
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());

		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	}

	//------------------------------- end navmesh code
	//End Rotation Control
    _footSteps.setPaused(false);
    return LEAVE;
}

int CBTRangedSushi::actionRetreat() {
	previousState = currentState;
	currentState = States::Retreat;
	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::WALK_LOOP, -0.5f);

	TCompName* cname = get<TCompName>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();

	//Movement Control
	Vector3 dir = Vector3();
	dir = -c_trans->getFront() * retreatSpeed * dt;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	//End Movement Control

	//Rotation Control
	//LookAt_Player(c_trans, player_position);
	c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
	//End Rotation Control
    _footSteps.setPaused(false);
    return LEAVE;
}

//int CBTRangedSushi::actionOrbitRight() {
//	previousState = currentState;
//	currentState = States::OrbitRight;
//	if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
//		return LEAVE;
//	}
//	if (_actionTimer > 0) {
//		CEntity* e_player = (CEntity *)h_player;
//		TCompTransform* p_trans = e_player->get<TCompTransform>();
//		TCompTransform* c_trans = get<TCompTransform>();
//		Vector3 p_pos = p_trans->getPosition();
//		Vector3 e_pos = c_trans->getPosition();		
//
//		float s = sin(-dt);
//		float c = cos(-dt);
//
//		// translate point back to origin:
//		e_pos.x -= p_pos.x;
//		e_pos.z -= p_pos.z;
//
//		// rotate point
//		float xnew = e_pos.x * c - e_pos.z * s;
//		float ynew = e_pos.x * s + e_pos.z * c;
//
//		// translate point back:
//		e_pos.x = xnew + p_pos.x;
//		e_pos.z = ynew + p_pos.z;
//
//		//Rotation Control
//		//LookAt_Player(c_trans, player_position);
//		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
//		//End Rotation Control		
//
//		//tranlacion de -EP
//		if (VEC3().Distance(p_pos, e_pos) < _orbitDistance) {
//			e_pos += (e_pos - p_pos) * _frontalSpeed * dt;
//		}
//		VEC3 dir = e_pos - c_trans->getPosition();
//		dir.Normalize();
//		dir = dir * _orbitSpeed;
//		dir *= dt;
//
//		TCompCollider* c_cc = get<TCompCollider>();
//		if (c_cc) {
//			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
//		}
//		return STAY;
//	}
//	else {
//		TCompName* cname = get<TCompName>();
//		dbg("%s leaves ORBIT_RIGHT\n", cname->getName());
//		//ChangeState("IDLEWAR");
//		_orbitCooldownTimer = _orbitCooldown;
//		return LEAVE;
//	}
//}
//
//int CBTRangedSushi::actionOrbitLeft() {
//	previousState = currentState;
//	currentState = States::OrbitLeft;
//	if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
//		return LEAVE;
//	}
//	if (_actionTimer > 0) {
//		CEntity* e_player = (CEntity *)h_player;
//		TCompTransform* p_trans = e_player->get<TCompTransform>();
//		TCompTransform* c_trans = get<TCompTransform>();
//		Vector3 p_pos = p_trans->getPosition();
//		Vector3 e_pos = c_trans->getPosition();
//
//		float s = sin(dt);
//		float c = cos(dt);
//
//		// translate point back to origin:
//		e_pos.x -= p_pos.x;
//		e_pos.z -= p_pos.z;
//
//		// rotate point
//		float xnew = e_pos.x * c - e_pos.z * s;
//		float ynew = e_pos.x * s + e_pos.z * c;
//
//		// translate point back:
//		//TODO: remove hardcoded extra distance
//		e_pos.x = xnew + p_pos.x;
//		e_pos.z = ynew + p_pos.z;
//
//		//Rotation Control
//		//LookAt_Player(c_trans, player_position);
//		c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
//		//End Rotation Control		
//
//		if (VEC3().Distance(p_pos, e_pos) < _orbitDistance) {
//			e_pos += (e_pos - p_pos) * _frontalSpeed * dt;
//		}
//		VEC3 dir = e_pos - c_trans->getPosition();
//		dir.Normalize();
//		dir = dir * _orbitSpeed;
//		dir *= dt;
//
//		TCompCollider* c_cc = get<TCompCollider>();
//		if (c_cc) {
//			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
//		}
//		return STAY;
//	}
//	else {
//		TCompName* cname = get<TCompName>();
//		dbg("%s leaves ORBIT_LEFT\n", cname->getName());
//		//ChangeState("IDLEWAR");
//		_orbitCooldownTimer = _orbitCooldown;
//		return LEAVE;
//	}
//}

int CBTRangedSushi::actionOnAir() {
	previousState = currentState;
	currentState = States::OnAir;
    _footSteps.setPaused(true);
    if (isGrounded() && impulse.y <= 0.0f || isDeadForFallout) {
		impulse.y = 0.0f;
		return LEAVE;
	}

	if (_hasBounced || _shootTimer <= 0)
		return LEAVE;

	TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
	sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_LOOP, 1.0f);

	//Movement Control
	impulse.x *= 1 - (0.05f * dt);
	impulse.z *= 1 - (0.05f * dt);

	VEC3 dir = impulse * dt;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	//End Movement Control
	return STAY;
}

int CBTRangedSushi::actionImpactReceived() {
	previousState = currentState;
	currentState = States::ImpactReceived;
	TCompName* cname = get<TCompName>();
	//dbg("%s receives an impact\n", cname->getName());

	if (life <= 0.f) {
		//ChangeState("DEAD");
		return LEAVE;
	}
	TCompTransform* c_trans = get<TCompTransform>();
	inCombat = true;
	combatViewDistance += Vector3::Distance(c_trans->getPosition(), damageSource) + 10.f;
	//Rotate towards player
	//LookAt_Player(c_trans, player_position, dt);
	//Move away from player for damageStunDuration seconds
	direction_to_damage *= impactForce;
	direction_to_damage *= dt;
	VEC3 dir = c_trans->getPosition() - damageSource;
	dir.Normalize();
	dir *= impactForce;
	dir *= dt;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc) {
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	}

	TCompRigidBody* c_rbody = get<TCompRigidBody>();

	if (c_rbody) {
		//c_rbody->addForce(direction_to_damage * impactForce);
	}
	damageStunTimer = -dt;
	return LEAVE;
}

int CBTRangedSushi::actionGravityReceived() {
	previousState = currentState;
	currentState = States::GravityReceived;
    _footSteps.setPaused(true);
    TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	inCombat = true;
	combatViewDistance += Vector3::Distance(c_trans->getPosition(), p_trans->getPosition()) + 10.f;
	TCompCollider* c_cc = get<TCompCollider>();
	c_cc->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	if (battery_time > 0) {
		TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
		sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_LOOP, 1.0f);

		//moverme hacia battery_position y si estoy en batery position no moverme
		VEC3 repulsionForce = VEC3().Zero;
		VEC3 attractionForce = VEC3().Zero;
		std::uniform_real_distribution<float> float_dist(-1.0, 1.0);
		//VEC3 randomForce = VEC3(0.0f, float_dist(bt_mt), 0.0f);
		//randomForce *= 65.f;
		VEC3 rotationForce = (Vector3().Up).Cross(c_trans->getPosition() - battery_position);
		rotationForce *= 4.f;


		float distance = VEC3::Distance(battery_position, c_trans->getPosition());
		dbg("Timer is %f\n", repulsionTimer);
		dbg("Distance is %f\n", distance);
		//If distance < distanceToBattery enable repulsion with repulsionTimer = repulsionDuration;
		if (distance < distanceToBattery) {
			repulsionTimer = repulsionDuration;
		}
		//Repel while distance <= distanceToBattery && repulsionTimer > 0
		//Attract while distance > distanceToBattery && repulsionTimer <= 0
		if (distance <= distanceToBattery || repulsionTimer > 0) {
			//Repulsion
			repulsionForce = c_trans->getPosition() - battery_position;
			repulsionForce.Normalize();
			float inverseDistance = 1 / distance;
			repulsionForce *= 10.f * inverseDistance;
			repulsionTimer -= dt;
			dbg("Repelling with force %f, %f, %f\n", repulsionForce.x, repulsionForce.y, repulsionForce.z);
		}
		else if (distance > distanceToBattery && repulsionTimer <= 0) {
			//Attraction
			attractionForce = battery_position - c_trans->getPosition();
			attractionForce.Normalize();
			float inverseDistance = 1 / distance;
			attractionForce *= 10.f;
			dbg("Attracting with force %f, %f, %f\n", attractionForce.x, attractionForce.y, attractionForce.z);
		}

		resultingForce += repulsionForce + attractionForce + rotationForce;
		resultingForce *= dt;
		battery_time -= dt;
		//switch between attraction and repulsion, a few frames each
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(resultingForce), 0.0f, dt, PxControllerFilters());
		}
		return STAY;
	}
	else {
		beingAttracted = false;
		dbg("BATTERY ends, sushi goes back to CHASE\n");
		c_cc->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
		resultingForce = Vector3().Zero;
		repulsionTimer = 0.f;
		//ChangeState("CHASE");
		return LEAVE;
	}

}

int CBTRangedSushi::actionFear() {
	previousState = currentState;
	currentState = States::FearReceived;
	if (_onFireArea) {
		_fearTimer = _fearDuration;
	}
	if (_fearTimer > 0.f) {
        _footSteps.setPaused(false);
        TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
		sushiAnimator->playAnimation(TCompSushiAnimator::WALK_LOOP, 2.0f);

		TCompTransform* c_trans = get<TCompTransform>();
		TCompCollider* c_cc = get<TCompCollider>();
		VEC3 fleeDirection = c_trans->getPosition() - _fearOrigin;
		fleeDirection.Normalize();
		fleeDirection += c_trans->getFront();
		fleeDirection *= _fearSpeed * dt;
		VEC3 nextPoint = c_trans->getPosition() + fleeDirection;
		VEC3 nextPointDirection = nextPoint - c_trans->getPosition();
		nextPointDirection.Normalize();
		nextPointDirection *= _fearSpeed * dt;
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(nextPointDirection), 0.0f, dt, PxControllerFilters());
		}
		c_trans->rotateTowards(nextPoint, rotationSpeed, dt);
		_fearTimer -= dt;
		return STAY;
	}
	else {
		return LEAVE;
	}
}

int CBTRangedSushi::actionDeath() {
	GameController.addEnemiesKilled(EntityType::SUSHI);

	//------------------------------------ Blackboard

	CEntity* e_player = (CEntity *)h_player;
	TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
	slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);

	if (slotsAvailable) {
		slotsAvailable = false;
		c_bb = e_player->get<TCompBlackboard>();
		c_bb->forgetPlayer(CHandle(this).getOwner(), SUSHI);
	}


	//------------------------------------
	TCompTransform* c_trans = get<TCompTransform>();
	if (!isDeadForFallout && !isDeadForTrigger) {

		GameController.spawnPuddle(c_trans->getPosition(), c_trans->getRotation(), 1.5f);
		TEntityParseContext ctx;
		ctx.root_transform = *c_trans;
		parseScene("data/prefabs/vfx/death_sphere.json", ctx);

		TCompSelfDestroy* c_sd = get<TCompSelfDestroy>();
		c_sd->setDelay(0.25f);
		c_sd->enable();
    TCompRigidBody* c_rb = get<TCompRigidBody>();
    c_rb->enableGravity(false);

		CEntity* portal = ctx.entities_loaded[0];
		CEntity* part_portal = ctx.entities_loaded[1];
		TCompDeathBillboard* c_db = portal->get<TCompDeathBillboard>();
		c_db->setParticles(part_portal);

		death_animation_started = true;
		CHandle h = GameController.entityByName("enemies_in_butcher");
		if (h.isValid()) {
			CEntity* enemies_in_butcher = ((CEntity*)h);
			TCompEnemiesInButcher* comp = enemies_in_butcher->get<TCompEnemiesInButcher>();
			if (comp != nullptr) {
				TMSgEnemyDead msgSushiDead;
				msgSushiDead.h_entity = CHandle(this).getOwner();
				msgSushiDead.isDead = true;
				enemies_in_butcher->sendMsg(msgSushiDead);
			}
		}


	}
	else {
		CHandle(this).getOwner().destroy();
		CHandle(this).destroy();
	}

  _footSteps.setPaused(true);
  return LEAVE;
}

int CBTRangedSushi::actionDecoy() {
	dbg("DECOY activated.\n");
	//Raycast behind me to check if there's room for me
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 origin = c_trans->getPosition();
	origin.y += 0.001f; //So we can detect collision with planes
	physx::PxRaycastHit hit;
	physx::PxFilterData pxFilterData;
	pxFilterData.word0 = EnginePhysics.Scenario;
	physx::PxQueryFilterData PxScenarioFilterData;
	PxScenarioFilterData.data = pxFilterData;
	PxScenarioFilterData.flags = physx::PxQueryFlag::eSTATIC;
	EnginePhysics.Raycast(origin, -c_trans->getFront(), _decoyTeleportDistance, hit, physx::PxQueryFlag::eSTATIC, PxScenarioFilterData);
	VEC3 ori = c_trans->getPosition();
	VEC3 posibleDestination = c_trans->getTranslatePositionForAngle(ori, _decoyTeleportDistance, 180);//si me voi hacia atras y hay vacio o pared no sigo retrocediendo
	VEC3 hit_navmesh = VEC3();
    bool r;
    if (use_navmesh) {
        r = EngineNavmesh.raycast(ori, posibleDestination, hit_navmesh);//true: hay interseccion limite navmesh
    }
    else {
        r = false;
    }
	if (hit.position != physx::PxVec3(0) || r) {
		//There is something behind me, can't teleport
		dbg("DECOY failed, obstacle behind.\n");
		_decoyTriggered = false;
		return LEAVE;
	}
	


	//At this point it's safe for us to teleport
	//Get decoy spawn point
	TCompCollider* c_cc = get<TCompCollider>();
	_decoyOrigin = origin;
	_decoyOrigin.y += c_cc->controller->getHeight();
	//Teleport
	VEC3 teleportPosition = origin + (-c_trans->getFront() * _decoyTeleportDistance);
	c_cc->controller->setPosition(VEC3_TO_PXEXVEC3(teleportPosition));
	//Smoke particles
	//Spawn the decoy in my old position
    std::string randN = to_string(rand() % 12 + 1);     // v2 in the range 1 to 100
    GameController.spawnPrefab("data/prefabs/products/product" + randN + ".json", _decoyOrigin);
    GameController.spawnPrefab("data/prefabs/vfx/smoke.json", _decoyOrigin);

    AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Ranged_Decoy");
    audio.set3DAttributes(*c_trans);
	//Smoke particles
	//Restore values
	_decoyTriggered = false;
	_decoyCooldownTimer = _decoyCooldown;
	_decoyOrigin = VEC3().Zero;
	return LEAVE;
}
#pragma endregion
//End Actions

//Conditions
#pragma region Conditions
bool CBTRangedSushi::conditionOnAir() {
	return !isGrounded() && !_hasBounced;
}

bool CBTRangedSushi::conditionPlayerInView() {
	/*if (player_dead) {
		inCombat = false;
		return false; //no lo ve si esta muerto
	}

	return isView();*/
	if (player_dead) {
		inCombat = false;
		return false; //no lo ve si esta muerto
	}

	bool res = false;
	if (isView() && checkHeight()) {//tiene que estar dentro de ambos rangos
		res = true;
		//return true;
	
	}
	
	if (use_navmesh) {
		//raycast de personaje hacia abajo
		if (!checkHeight()) {
			inCombat = false;
			return false;
		}
		if(isView()) {
			isPlayerInNavmesh();
			if (navmeshPath.size() == 0) {
				res = false;
				initialExecution = true;
				hayCamino = false;
			}
			else {
				hayCamino = true;
				//inCombat = false;
			}
		}
	}
	return res;

}

bool CBTRangedSushi::conditionSalute() {
	return ((previousState == States::SeekWaypoint || previousState == States::NextWaypoint) && !hasSaluted && (bt_dist_rs(bt_mt_rs) < saluteProbability));
}
bool CBTRangedSushi::conditionLeap() {
	return (isGrounded() && rollDice(_leapProbability) && checkBlackboard()) || _hasBounced;
}
bool CBTRangedSushi::conditionBounce() {
	return _hasBounced;
}

//If the player is too far
bool CBTRangedSushi::conditionChase() {
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	return (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) > shootRange);
}

//If the player is too close
bool CBTRangedSushi::conditionRetreat() {
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	if (use_navmesh) {
		VEC3 ori = c_trans->getPosition();
		VEC3 posibleDestination = c_trans->getTranslatePositionForAngle(ori, 1, 180);//si me voi hacia atras y hay vacio o pared no sigo retrocediendo
		VEC3 hit = VEC3();
		bool r = EngineNavmesh.raycast(ori, posibleDestination, hit);
		if (r) {
			return false;
		}

	}
	return (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) <= retreatDistance);
}

//If the player is within range and we can do an action
bool CBTRangedSushi::conditionShoot() {
	return (_shootTimer <= 0 && checkBlackboard());
}

//If the player is within range and orbit is out of cooldown
//bool CBTRangedSushi::conditionOrbit() {
//	TCompTransform* c_trans = get<TCompTransform>();
//	CEntity* e_player = (CEntity *)h_player;
//	TCompTransform* p_trans = e_player->get<TCompTransform>();
//	return (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) <= shootRange) && _orbitCooldownTimer <= 0;
//}

bool CBTRangedSushi::conditionImpactReceived() {
	return damageStunTimer > 0.f;
}

bool CBTRangedSushi::conditionFear() {
	return _onFireArea;
}

bool CBTRangedSushi::conditionGravityReceived() {
	return beingAttracted && battery_time > 0;
}

bool CBTRangedSushi::conditionDeath() {
	return life <= 0.f && !death_animation_started;
}

bool CBTRangedSushi::conditionDecoy() {
	return _decoyTriggered;
}
#pragma endregion
//End Conditions


/* --------------------- Helper Functions ---------------------*/
#pragma region Helper Functions

void CBTRangedSushi::generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc)
{
	navmeshPath = EngineNavmesh.findPath(initPos, destPos);
	navmeshPathPoint = 0;
	recalculateNavmesh = recalc;

}

bool CBTRangedSushi::rollDice(int probability) {
	if (_actionTimer <= 0) {
		_actionTimer = _actionDelay;
		return (bt_dist_rs(bt_mt_rs) < probability);
	}
	else {
		return false;
	}
}

template <typename T>
void CBTRangedSushi::Send_DamageMessage(CEntity* entity, float dmg) {
	//Send damage mesage
	T msg;
	// Who sent this bullet
	msg.h_sender = h_sender;
	msg.h_bullet = CHandle(this).getOwner();
	msg.intensityDamage = dmg;
	entity->sendMsg(msg);
}

bool CBTRangedSushi::isView() {
	CEntity* e_player = (CEntity *)h_player;
	if (e_player == nullptr) {
		return false;
	}
	TCompTransform* player_position = e_player->get<TCompTransform>();
	TCompTransform* c_trans = get<TCompTransform>();
	float distance = Vector3::Distance(c_trans->getPosition(), player_position->getPosition());
	if (!inCombat) {
		//si no estamos en combate, view es dentro del cono y a menos distancia que viewDistance
		//o
		//a menos distancia que hearing_radius
		float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
		bool sighted = ((abs(angle) <= half_cone) && (distance <= viewDistance)) || distance <= hearing_radius;
		
		if (use_navmesh) {
			if (sighted && hayCamino)
				inCombat = true;
			return sighted;
		}
		else {
			if (sighted)
				inCombat = true;
			return sighted;
		}
	}
	else {
		//si estamos en combate, view es menos distancia que combatViewDistance
		if (use_navmesh) {
			bool sighted = (distance <= combatViewDistance) && hayCamino;
			if (!sighted)
				inCombat = false;
			return sighted;
		}
		else {
			bool sighted = (distance <= combatViewDistance);
			if (!sighted)
				inCombat = false;
			return sighted;
		}
	}
	return false;
}

bool CBTRangedSushi::isGrounded() {
	TCompRigidBody* r_body = get<TCompRigidBody>();
	if (r_body == nullptr) {//Si no existe el rigid body devuelvo false (no peta ya)
		return false;
	}
	return r_body->is_grounded;
	//return r_body->is_grounded;
}

void CBTRangedSushi::OnAir() {
	if (isGrounded()) {
		//ChangeState(last_state);
		return;
	}

	//Movement Control
	impulse.x *= 1 - (0.05f * dt);
	impulse.z *= 1 - (0.05f * dt);

	VEC3 dir = impulse * dt;
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc)
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	//End Movement Control
}

#pragma endregion
/* -------------------- End Helper Functions --------------------*/


void CBTRangedSushi::load(const json& j, TEntityParseContext& ctx) {
	//Pathing
	nWaypoints = j.value("nWaypoints", nWaypoints);
	//auto k = j["waypoints"];

	if (j.count("waypoints") > 0) {
		const json& jpositions = j["waypoints"];
		for (const json& i : j["waypoints"]) {
			VEC3 pos = loadVEC3(i);
			positions.push_back(pos);
		}
	}

	//End Pathing

	_initiallyPaused = j.value("_initiallyPaused", _initiallyPaused);

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

	//Salute Values
	saluteProbability = j.value("saluteProbability", saluteProbability);
	saluteDuration = j.value("saluteDuration", saluteDuration);
	//End Salute Values

	//Orbit Values
	/*_orbitProbability = j.value("_orbitProbability", _orbitProbability);
	_orbitSpeed = j.value("_orbitSpeed", _orbitSpeed);
	_frontalSpeed = j.value("_frontalSpeed", _frontalSpeed);
	_orbitDistance = j.value("_orbitDistance", _orbitDistance);*/
	//End Orbit Values

	//Fear Values
	_fearSpeed = j.value("_fearSpeed", _fearSpeed);
	_fearDuration = j.value("orbitSpeed", _fearDuration);
	//End Fear Values

	//Decoy Values
	_decoyCooldown = j.value("_decoyCooldown", _decoyCooldown);
	_decoyProbability = j.value("_decoyProbability", _decoyProbability);
	_decoyTeleportDistance = j.value("_decoyTeleportDistance", _decoyTeleportDistance);
	//End Decoy Values

	//navmesh values 
	use_navmesh = j.value("use_navmesh", use_navmesh);
	reevaluatePathDelay = j.value("reevaluatePathDelay", reevaluatePathDelay);
	//end navmesh values

	//define curve
	if (j.count("curve") > 0) {
		_curve = Resources.get(j.value("curve", ""))->as<CCurve>();
	}



	this->create("bt_sushi");
}

void CBTRangedSushi::renderDebug() {
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
		drawCircle(pos, shootRange, VEC4(1, 0, 0, 1));
		drawCircle(pos, hearing_radius, VEC4(1, 0, 0, 1));
		drawLine(pos, pos + half_cone_1 * combatViewDistance, VEC4(1, 0, 0, 1));
		drawLine(pos, pos + half_cone_2 * combatViewDistance, VEC4(1, 0, 0, 1));
		drawLine(pos + half_cone_1 * combatViewDistance, pos + half_cone_2 * combatViewDistance, VEC4(1, 0, 0, 1));

	}


	CHandle player = GameController.getPlayerHandle();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* trans_p = e_player->get<TCompTransform>();

	/*VEC3 directionToPlayer = c_trans->getPosition() - trans_p->getPosition();
	if (directionToPlayer.z < 0) {
		VEC3 char_pos_right = c_trans->getPosition();
		char_pos_right.y = 0.5f;
		char_pos_right.x = c_trans->getPosition().x - 0.5f;
		PxVec3 origin_right = VEC3_TO_PXVEC3(char_pos_right);

		VEC3 char_pos_left = c_trans->getPosition();
		char_pos_left.y = 0.5f;
		char_pos_left.x = c_trans->getPosition().x + 0.5f;
		PxVec3 origin_left = VEC3_TO_PXVEC3(char_pos_left);
		drawLine(PXVEC3_TO_VEC3(origin_right), (PXVEC3_TO_VEC3(origin_right) + PXVEC3_TO_VEC3(c_trans->getRight()) * 10), VEC4(0, 1, 1, 1));
		drawLine(PXVEC3_TO_VEC3(origin_left), (PXVEC3_TO_VEC3(origin_left) + PXVEC3_TO_VEC3(c_trans->getLeft()) * 10), VEC4(1, 1, 0, 1));

	}
	else {
		VEC3 char_pos_right = c_trans->getPosition();
		char_pos_right.y = 0.5f;
		char_pos_right.x = c_trans->getPosition().x + 0.5f;
		PxVec3 origin_right = VEC3_TO_PXVEC3(char_pos_right);

		VEC3 char_pos_left = c_trans->getPosition();
		char_pos_left.y = 0.5f;
		char_pos_left.x = c_trans->getPosition().x - 0.5f;
		PxVec3 origin_left = VEC3_TO_PXVEC3(char_pos_left);
		drawLine(PXVEC3_TO_VEC3(origin_right), (PXVEC3_TO_VEC3(origin_right) + PXVEC3_TO_VEC3(c_trans->getRight()) * 10), VEC4(0, 1, 1, 1));
		drawLine(PXVEC3_TO_VEC3(origin_left), (PXVEC3_TO_VEC3(origin_left) + PXVEC3_TO_VEC3(c_trans->getLeft()) * 10), VEC4(1, 1, 0, 1));
	}*/

	TCompCollider* coll_p = e_player->get<TCompCollider>();
	
	
	

	VEC3 char_pos_right = c_trans->getPosition();
	char_pos_right.y = 0.f;
	char_pos_right.x = c_trans->getPosition().x + coll_p->controller->getRadius();

	VEC3 char_pos_left = c_trans->getPosition();
	char_pos_left.y = 0.f;
	char_pos_left.x = c_trans->getPosition().x - coll_p->controller->getRadius();
	PxVec3 origin_right = VEC3_TO_PXVEC3(char_pos_right);
	PxVec3 origin_left = VEC3_TO_PXVEC3(char_pos_left);
	drawLine(PXVEC3_TO_VEC3(origin_right), (PXVEC3_TO_VEC3(origin_right) + PXVEC3_TO_VEC3(c_trans->getRight()) * 10), VEC4(0, 1, 1, 1));
	drawLine(PXVEC3_TO_VEC3(origin_left), (PXVEC3_TO_VEC3(origin_left) + PXVEC3_TO_VEC3(c_trans->getLeft()) * 10), VEC4(1, 1, 0, 1));
	
	
	

}

void CBTRangedSushi::onBlackboardMsg(const TMsgBlackboard& msg) {
	player_dead = msg.player_dead;

	if (player_dead) {
		inCombat = false;
	}
}

void CBTRangedSushi::registerMsgs() {
	DECL_MSG(CBTRangedSushi, TMsgBlackboard, onBlackboardMsg);
	DECL_MSG(CBTRangedSushi, TMsgFireAreaEnter, onFireAreaEnter);
	DECL_MSG(CBTRangedSushi, TMsgFireAreaExit, onFireAreaExit);
	DECL_MSG(CBTRangedSushi, TMsgDamage, onGenericDamageInfoMsg);
	DECL_MSG(CBTRangedSushi, TMsgOnContact, onCollision);
	DECL_MSG(CBTRangedSushi, TMsgGravity, onGravity);
	DECL_MSG(CBTRangedSushi, TMsgBTPaused, onMsgBTPaused);
	DECL_MSG(CBTRangedSushi, TMSgTriggerFalloutDead, onTriggerFalloutDead);
	DECL_MSG(CBTRangedSushi, TMsgDeleteTrigger, onDeleteTrigger);
}

void CBTRangedSushi::onGenericDamageInfoMsg(const TMsgDamage& msg) {

	if (isPaused()) {
		return;
	}
	if (msg.targetType & EntityType::SUSHI) {

		h_sender = msg.h_sender;
		damageSource = msg.position;
		life -= msg.intensityDamage;
		TCompTransform* my_trans = get<TCompTransform>();
		VEC3 direction_to_damage;

		//Decoy check
		if ((bt_dist_rs(bt_mt_rs) < _decoyProbability && _decoyCooldownTimer <= 0.f)) {
			_decoyTriggered = true;
			return;
		}

		if (msg.senderType == PLAYER) {
			CEntity* entity_to_hit_me = (CEntity *)msg.h_bullet;
			TCompTransform* e_trans = entity_to_hit_me->get<TCompTransform>();
			direction_to_damage = my_trans->getPosition() - e_trans->getPosition();
		}
		else {
			direction_to_damage = my_trans->getPosition() - msg.position;
		}

		direction_to_damage.y = 1.0f;
		direction_to_damage.Normalize();
		TCompRigidBody* c_rbody = get<TCompRigidBody>();
		if (c_rbody) {
			c_rbody->addForce(direction_to_damage * msg.impactForce);
		}
		if (life < 0) {
			life = 0;
		}
		damageStunTimer = damageStunDuration;
		FluidDecalGenerator.generateFluid(msg.impactForce, my_trans->getPosition());
	}
}

void CBTRangedSushi::onCollision(const TMsgOnContact& msg) {
	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	if (source_of_impact) {
		TCompTags* c_tag = source_of_impact->get<TCompTags>();
		if (c_tag) {
			std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
			//If I collide with the player while I'm charging, I send TMsgDamageToPlayer
			if (currentState == States::Leap && _isLeaping && tag != "floor") {
				_hasBounced = true;
				_bounceSource = msg.pos;
			}
		}
	}
}

void CBTRangedSushi::onGravity(const TMsgGravity& msg) {
	if (!beingAttracted && msg.time_effect > 0) {
		h_sender = msg.h_sender;
		h_bullet = msg.h_bullet;
		//obtener posicion
		beingAttracted = true;
		CEntity* e_bullet = (CEntity *)h_bullet;
		TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
		battery_position = msg.position;
		battery_time = msg.time_effect;
		attractionForce = msg.attractionForce;
		distanceToBattery = msg.distance;

		//ChangeState("BATTERYEFFECT");
	}if (beingAttracted && h_bullet == msg.h_bullet) {

		//obtener posicion
		beingAttracted = true;
		CEntity* e_bullet = (CEntity *)h_bullet;
		TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
		battery_position = msg.position;

		//ChangeState("BATTERYEFFECT");
	}
}

void CBTRangedSushi::onFireAreaEnter(const TMsgFireAreaEnter& msg) {
	dbg("Sushi stepped in the fire!\n");
	_onFireArea = true;
	_fearTimer = _fearDuration;
	_fearOrigin = msg.areaCenter;
}
void CBTRangedSushi::onFireAreaExit(const TMsgFireAreaExit& msg) {
	dbg("Sushi got out of the fire\n");
	_onFireArea = false;
}


void CBTRangedSushi::onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg) {
	life -= msg.damage;
	isDeadForFallout = msg.falloutDead;
	if (life < 0) {
		life = 0;
	}
}


std::string CBTRangedSushi::getState() {
	switch (currentState) {
	case States::undefined:
		return "undefined";
		break;
	case States::NextWaypoint:
		return "NextWaypoint";
		break;
	case States::SeekWaypoint:
		return "SeekWaypoint";
		break;
	case States::Salute:
		return "Salute";
		break;
	case States::Shoot:
		return "Shoot";
		break;
	case States::Leap:
		return "Leap";
		break;
	case States::Bounce:
		return "Bounce";
		break;
	case States::Chase:
		return "Chase";
		break;
	case States::OrbitRight:
		return "OrbitRight";
		break;
	case States::OrbitLeft:
		return "OrbitLeft";
		break;
	case States::OnAir:
		return "OnAir";
		break;
	case States::ImpactReceived:
		return "ImpactReceived";
		break;
	case States::GravityReceived:
		return "GravityReceived";
		break;
	case States::FearReceived:
		return "FearReceived";
		break;
	}
}

bool CBTRangedSushi::checkBlackboard() {
	CEntity* e_player = (CEntity *)h_player;
	TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
	slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);
	return slotsAvailable;
}

void CBTRangedSushi::debugInMenu() {
	//Core Values
	//ImGui::TreeNode("Core Values");
	ImGui::Checkbox("_pausedBT", &_pausedBT);

}

bool CBTRangedSushi::isHole(VEC3 direction) {
    if (!use_navmesh) {
        return true;
    }

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 char_pos = c_trans->getPosition();
	char_pos.y = 2;
	PxVec3 origin = VEC3_TO_PXVEC3(char_pos);
	PxVec3 unitDir = VEC3_TO_PXVEC3(c_trans->getRight());
	if (directionJump == 1) {//LEFT
		unitDir = VEC3_TO_PXVEC3(c_trans->getLeft());
		//dbg("Salto a la izquierda\n");
	}
	else {

		//dbg("Salto a la derecha\n");
	}
	PxReal maxDistance = 10.0f;
	PxRaycastBuffer hit;
	PxRaycastHit hitBuffer[10];
	hit = PxRaycastBuffer(hitBuffer, 10);

	// [in] Define what parts of PxRaycastHit we're interested in
	const PxHitFlags outputFlags =
		PxHitFlag::eDISTANCE
		| PxHitFlag::ePOSITION
		| PxHitFlag::eNORMAL
		;

	/*bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit, outputFlags);

	if (res) {//colisiona con algo
		int closestIdx = -1;
		float closestDist = 1000.0f;
		dbg("Number of hits: %i \n", hit.getNbAnyHits());
		for (int i = 0; i < hit.getNbAnyHits(); i++) {
			if (hit.getAnyHit(i).distance <= closestDist) {
				closestDist = hit.getAnyHit(i).distance;
				closestIdx = i;
			}
		}
		if (closestIdx != -1) {
			CHandle hitCollider;
			PxShape* colShape;
			for (int i = 0; i < hit.getAnyHit(closestIdx).actor->getNbShapes(); i++) {
				hit.getAnyHit(closestIdx).actor->getShapes(&colShape, 1, i);
				PxFilterData col_filter_data = colShape->getSimulationFilterData();
				if (col_filter_data.word0 & !EnginePhysics.Enemy ) {
					hitCollider.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
					if (hitCollider.isValid()) {
						CEntity* candidate = hitCollider.getOwner();
						dbg("el candidato obj es valido nombre = %s  \n", candidate->getName());
						return true;
					}
				}
			}
		}

	}
	else {//no colisiono*/
	/*float dis =  EngineNavmesh.wallDistance(c_trans->getPosition());
	dbg("DIST:%f\n", dis);

	if(EngineNavmesh.wallDistance(char_pos) >= 1.f){
		return true;
	}*/
	//calcular hacia donde te mira
	CHandle player = GameController.getPlayerHandle();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* trans_p = e_player->get<TCompTransform>();

	VEC3 directionToPlayer = c_trans->getPosition() - trans_p->getPosition();
	//dbg("ALGO: x:%f,y:%f,z:%f\n", directionToPlayer.x, directionToPlayer.y, directionToPlayer.z);

	if (directionToPlayer.z < 0) {
		if (directionJump == 1) {//LEFT
			VEC3 ori = c_trans->getPosition();
			VEC3 posibleDestination = c_trans->getTranslatePositionForAngle(ori, 10, 90);//miro a mi derecha
			VEC3 hit = VEC3();
			bool r = EngineNavmesh.raycast(ori, posibleDestination, hit);
			if (!r) {
				//dbg("CASO 1\n");
				return true;
			}
			else {
				//dbg("CASO 2\n");
				return false;
			}
		}
		else {
			VEC3 ori = c_trans->getPosition();
			VEC3 posibleDestination = c_trans->getTranslatePositionForAngle(ori, 10, -90);//miro a mi derecha
			VEC3 hit = VEC3();
			bool r = EngineNavmesh.raycast(ori, posibleDestination, hit);
			if (!r) {
				//dbg("CASO 3\n");
				return true;
			}
			else {
				//dbg("CASO 4\n");
				return false;
			}
		}
	}
	else {
		if (directionJump == 1) {//LEFT
			VEC3 ori = c_trans->getPosition();
			VEC3 posibleDestination = c_trans->getTranslatePositionForAngle(ori, 10, 90);//miro a mi derecha
			VEC3 hit = VEC3();
			bool r = EngineNavmesh.raycast(ori, posibleDestination, hit);
			if (!r) {
				//dbg("CASO 5\n");
				return true;
			}
			else {
			//	dbg("CASO 6\n");
				return false;
			}
		}
		else {
			VEC3 ori = c_trans->getPosition();
			VEC3 posibleDestination = c_trans->getTranslatePositionForAngle(ori, 10, -90);//miro a mi derecha
			VEC3 hit = VEC3();
			bool r = EngineNavmesh.raycast(ori, posibleDestination, hit);
			if (!r) {
				//dbg("CASO 7\n");
				return true;
			}
			else {
				//dbg("CASO 8\n");
				return false;
			}
		}

	}
}


VEC3 CBTRangedSushi::calculatePositionGround() {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();

	VEC3 char_pos = p_trans->getPosition();
	VEC3 positionJump = char_pos;
	PxReal maxDistance = 10.0f;
	PxRaycastBuffer hit;
	PxRaycastHit hitBuffer[10];
	hit = PxRaycastBuffer(hitBuffer, 10);
	const PxHitFlags outputFlags =
		PxHitFlag::eDISTANCE
		| PxHitFlag::ePOSITION
		| PxHitFlag::eNORMAL;
	TCompTransform* c_trans = get<TCompTransform>();

	
	PxVec3 origin = VEC3_TO_PXVEC3(char_pos);
	PxVec3 unitDir = VEC3_TO_PXVEC3((-c_trans->getUp()));//direccion abajo
	bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit, outputFlags);
	if (res) {//colisiona con algo
		int closestIdx = -1;
		float closestDist = 1000.0f;
		//dbg("Number of hits: %i \n", hit.getNbAnyHits());
		for (int i = 0; i < hit.getNbAnyHits(); i++) {
			if (hit.getAnyHit(i).distance <= closestDist) {
				closestDist = hit.getAnyHit(i).distance;
				closestIdx = i;
			}
		}
		if (closestIdx != -1) {
			CHandle hitCollider;
			PxShape* colShape;
			for (int i = 0; i < hit.getAnyHit(closestIdx).actor->getNbShapes(); i++) {
				hit.getAnyHit(closestIdx).actor->getShapes(&colShape, 1, i);
				PxFilterData col_filter_data = colShape->getSimulationFilterData();
				if (col_filter_data.word0 & EnginePhysics.All) {
					hitCollider.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
					if (hitCollider.isValid()) {
						CEntity* candidate = hitCollider.getOwner();
						if(candidate != nullptr){
							//dbg("el candidato obj es valido nombre = %s  \n", candidate->getName());
						}
						positionJump = PXVEC3_TO_VEC3(hit.getAnyHit(closestIdx).position);
					}
				}
			}
		}
	}
	//dbg("positionJump.x:%f,positionJump.y:%fpositionJump.z:%f\n", positionJump.x, positionJump.y, positionJump.z);
	
	return positionJump;
}



bool CBTRangedSushi::obstacleInJump(){
	PxReal maxDistance = 10.0f;
	PxRaycastBuffer hit;
	PxRaycastHit hitBuffer[10];
	hit = PxRaycastBuffer(hitBuffer, 10);
	// [in] Define what parts of PxRaycastHit we're interested in
	const PxHitFlags outputFlags =
		PxHitFlag::eDISTANCE
		| PxHitFlag::ePOSITION
		| PxHitFlag::eNORMAL
		;

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 char_pos = c_trans->getPosition();
	char_pos.y = 2;
	PxVec3 origin = VEC3_TO_PXVEC3(char_pos);
	PxVec3 unitDir = VEC3_TO_PXVEC3(c_trans->getRight());
	if (directionJump == 1) {//LEFT
		unitDir = VEC3_TO_PXVEC3(c_trans->getLeft());
		
	}
	bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit, outputFlags);
	if (res) {//colisiona con algo
		int closestIdx = -1;
		float closestDist = 1000.0f;
		//dbg("Number of hits: %i \n", hit.getNbAnyHits());
		for (int i = 0; i < hit.getNbAnyHits(); i++) {
			if (hit.getAnyHit(i).distance <= closestDist) {
				closestDist = hit.getAnyHit(i).distance;
				closestIdx = i;
			}
		}
		if (closestIdx != -1) {
			CHandle hitCollider;
			PxShape* colShape;
			for (int i = 0; i < hit.getAnyHit(closestIdx).actor->getNbShapes(); i++) {
				hit.getAnyHit(closestIdx).actor->getShapes(&colShape, 1, i);
				PxFilterData col_filter_data = colShape->getSimulationFilterData();
				if (col_filter_data.word0 & EnginePhysics.All ) {
					hitCollider.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
					if (hitCollider.isValid()) {
						CEntity* candidate = hitCollider.getOwner();
						dbg("el candidato obj es valido nombre = %s  \n", candidate->getName());
						return true;
					}
				}
			}
		}

	}
	return false;
}

bool CBTRangedSushi::isPlayerInNavmesh() {

	//CEntity* e_player = (CEntity *)h_player;
	//TCompTransform* p_trans = e_player->get<TCompTransform>();
	VEC3 posPlayer = calculatePositionGround();
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 position = c_trans->getPosition();
	//wtp 
	generateNavmesh(position, posPlayer, false);
	
	//inCombat = false;
	return true;
}


bool CBTRangedSushi::checkHeight() {
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


void CBTRangedSushi::setHeightRange(float height) {
  this->height_range = height;
}
std::string CBTRangedSushi::getNameCurve() {
	return pathCurve;
}

void CBTRangedSushi::setViewDistance(float value) {
	this->viewDistance = value;
}

void CBTRangedSushi::setHalfCone(float halfCone) {
	this->half_cone = halfCone;
}


bool CBTRangedSushi::isOtherEnemyInSide() {
	bool hayEnemy = false;
	PxReal maxDistance = 10.f;
	PxRaycastBuffer hit;
	PxRaycastHit hitBuffer[10];
	hit = PxRaycastBuffer(hitBuffer, 10);
	// [in] Define what parts of PxRaycastHit we're interested in
	const PxHitFlags outputFlags =
		PxHitFlag::eDISTANCE
		| PxHitFlag::ePOSITION
		| PxHitFlag::eNORMAL
		;

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 char_pos_right = c_trans->getPosition();
	char_pos_right.y = 0.5f;
	PxVec3 origin_right = VEC3_TO_PXVEC3(char_pos_right);
	PxVec3 unitDirRight = VEC3_TO_PXVEC3(c_trans->getRight());
	PxVec3 unitDirLeft = VEC3_TO_PXVEC3(c_trans->getLeft());
	PxQueryFilterData filter_data = PxQueryFilterData();
	filter_data.data.word0 = EnginePhysics.Enemy;

	//se mira a la izquierda si hay un enemigo 
	
	bool res = EnginePhysics.gScene->raycast(origin_right, unitDirRight, maxDistance, hit, outputFlags, filter_data);
	if (res) {//colisiona con algo
		int closestIdx = -1;
		float closestDist = 1000.0f;
		//dbg("Number of hits: %i \n", hit.getNbAnyHits());
		for (int i = 0; i < hit.getNbAnyHits(); i++) {
			

			if (hit.getAnyHit(i).distance <= closestDist) {
				closestDist = hit.getAnyHit(i).distance;
				closestIdx = i;
			}
		}
		if (closestIdx != -1) {
			
			PxShape* colShape;
			for (int i = 0; i < hit.getAnyHit(closestIdx).actor->getNbShapes(); i++) {
				hit.getAnyHit(closestIdx).actor->getShapes(&colShape, 1, i);
				CHandle h_comp_physics;
				h_comp_physics.fromVoidPtr(hit.getAnyHit(i).actor->userData);
				CEntity* entityContact = h_comp_physics.getOwner();
				dbg("Entidad con quien choco--->%s\n", entityContact->getName());
				

				PxFilterData col_filter_data = colShape->getSimulationFilterData();
				CEntity* mine = CHandle(this).getOwner();
				dbg("YO--->%s\n", mine->getName());
				
				if (col_filter_data.word0 & EnginePhysics.Enemy && entityContact != (mine)) {
					CHandle hitCollider;
					hitCollider.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
					if (hitCollider.isValid()) {
						CEntity* candidate = hitCollider.getOwner();
						dbg("el candidato obj es valido nombre = %s  \n", candidate->getName());
						hayEnemy = true;
					}
				}
			}
		}
	}


	//se mira a la izquierda si hay un enemigo 
	VEC3 char_pos_left = c_trans->getPosition();
	char_pos_left.y = 0.5f;
	
	PxVec3 origin_left = VEC3_TO_PXVEC3(char_pos_left);
	res = EnginePhysics.gScene->raycast(origin_left, unitDirLeft, maxDistance, hit, outputFlags, filter_data);
	if (res) {//colisiona con algo
		int closestIdx = -1;
		float closestDist = 1000.0f;
		//dbg("Number of hits: %i \n", hit.getNbAnyHits());
		for (int i = 0; i < hit.getNbAnyHits(); i++) {


			if (hit.getAnyHit(i).distance <= closestDist) {
				closestDist = hit.getAnyHit(i).distance;
				closestIdx = i;
			}
		}
		if (closestIdx != -1) {

			PxShape* colShape;
			for (int i = 0; i < hit.getAnyHit(closestIdx).actor->getNbShapes(); i++) {
				hit.getAnyHit(closestIdx).actor->getShapes(&colShape, 1, i);
				CHandle h_comp_physics;
				h_comp_physics.fromVoidPtr(hit.getAnyHit(i).actor->userData);
				CEntity* entityContact = h_comp_physics.getOwner();
				dbg("Entidad con quien choco--->%s\n", entityContact->getName());


				PxFilterData col_filter_data = colShape->getSimulationFilterData();
				CEntity* mine = CHandle(this).getOwner();
				dbg("YO--->%s\n", mine->getName());

				if (col_filter_data.word0 & EnginePhysics.Enemy && entityContact != (mine)) {
					CHandle hitCollider;
					hitCollider.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
					if (hitCollider.isValid()) {
						CEntity* candidate = hitCollider.getOwner();
						dbg("el candidato obj es valido nombre = %s  \n", candidate->getName());
						hayEnemy = true;
					}
				}
			}
		}
	}
	

	return hayEnemy;

}


void CBTRangedSushi::onDeleteTrigger(const TMsgDeleteTrigger& msg) {
	isDeadForTrigger = true;
	life = 0;
}
