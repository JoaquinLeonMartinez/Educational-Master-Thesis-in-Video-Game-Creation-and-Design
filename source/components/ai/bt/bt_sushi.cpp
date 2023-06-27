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
#include "components/objects/comp_enemies_in_butcher.h"
#include "bt_sushi.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/ai/others/self_destroy.h"
#include "components/vfx/comp_death_billboard.h"

#include "random"

using namespace physx;
std::mt19937 bt_mt(1729);
std::uniform_int_distribution<int> bt_dist(0, 100);

DECL_OBJ_MANAGER("bt_sushi", CBTSushi);
void CBTSushi::create(string s)//crear el arbol
{

    if (!h_player.isValid()) {
        h_player = GameController.getPlayerHandle();
    }

    if (_initiallyPaused) {
        setPaused(true);
    }


    name = s; //createRoot y addChild debe hacerse en orden, sino peta -- nunca definir un huerfano directamente 
              // padre - hijo - tipo - condition - action
    createRoot("SUSHI", PRIORITY, NULL, NULL);
    addChild("SUSHI", "ON_DEATH", ACTION, (btcondition)&CBTSushi::conditionDeath, (btaction)&CBTSushi::actionDeath);
    addChild("SUSHI", "DEATH", ACTION, (btcondition)& CBTSushi::conditionDeathAnimation, (btaction)& CBTSushi::actionDeathStay);
    addChild("SUSHI", "ON_GRAVITY", ACTION, (btcondition)&CBTSushi::conditionGravityReceived, (btaction)&CBTSushi::actionGravityReceived);
    addChild("SUSHI", "ON_FEAR", ACTION, (btcondition)&CBTSushi::conditionFear, (btaction)&CBTSushi::actionFear);
    addChild("SUSHI", "ON_IMPACT", ACTION, (btcondition)&CBTSushi::conditionImpactReceived, (btaction)&CBTSushi::actionImpactReceived);
    addChild("SUSHI", "ON_AIR", ACTION, (btcondition)&CBTSushi::conditionOnAir, (btaction)&CBTSushi::actionOnAir);

    addChild("SUSHI", "VIEW", PRIORITY, (btcondition)& CBTSushi::conditionPlayerInView, NULL);
    //addChild("VIEW", "SALUTE", ACTION, (btcondition)&CBTSushi::conditionSalute, (btaction)&CBTSushi::actionSalute);
    addChild("VIEW", "COMBAT_HOLDER", PRIORITY, NULL, NULL);

    addChild("SUSHI", "PATROL", SEQUENCE, NULL, NULL);
    addChild("PATROL", "NEXT_WAYPOINT", ACTION, NULL, (btaction)& CBTSushi::actionNextWaypoint);
    addChild("PATROL", "SEEK_WAYPOINT", ACTION, NULL, (btaction)& CBTSushi::actionSeekWaypoint);

    addChild("COMBAT_HOLDER", "MELEE_HOLDER", PRIORITY, (btcondition)& CBTSushi::conditionMelee, NULL);
    addChild("MELEE_HOLDER", "BLOCK_HOLDER", PRIORITY, (btcondition)& CBTSushi::conditionBlock, NULL);
    //addChild("BLOCK_HOLDER", "ORBIT_HOLDER", RANDOM, (btcondition)&CBTSushi::conditionBlockOrbit, NULL);
    addChild("BLOCK_HOLDER", "BLOCK", ACTION, NULL, (btaction)& CBTSushi::actionBlock);
    //addChild("ORBIT_HOLDER", "BLOCK_ORBIT_RIGHT", ACTION, NULL, (btaction)&CBTSushi::actionBlockOrbitRight);
    //addChild("ORBIT_HOLDER", "BLOCK_ORBIT_LEFT", ACTION, NULL, (btaction)&CBTSushi::actionBlockOrbitLeft);

    addChild("MELEE_HOLDER", "COMBO_HOLDER", PRIORITY, NULL, NULL);
    addChild("COMBO_HOLDER", "MELEE2", ACTION, (btcondition)& CBTSushi::conditionCombo2, (btaction)& CBTSushi::actionMelee2);
    addChild("COMBO_HOLDER", "MELEE3", ACTION, (btcondition)& CBTSushi::conditionCombo3, (btaction)& CBTSushi::actionMelee3);
    addChild("COMBO_HOLDER", "MELEE1", ACTION, NULL, (btaction)& CBTSushi::actionMelee1);

    addChild("COMBAT_HOLDER", "CHARGE_HOLDER", SEQUENCE, (btcondition)& CBTSushi::conditionCharge, NULL);
    addChild("CHARGE_HOLDER", "PREPARE_CHARGE", ACTION, NULL, (btaction)& CBTSushi::actionPrepareCharge);
    addChild("CHARGE_HOLDER", "CHARGE", ACTION, NULL, (btaction)& CBTSushi::actionCharge);

    addChild("COMBAT_HOLDER", "JUMPCHARGE_HOLDER", SEQUENCE, (btcondition)& CBTSushi::conditionJumpCharge, NULL);
    addChild("JUMPCHARGE_HOLDER", "PREPARE_JUMPCHARGE", ACTION, NULL, (btaction)& CBTSushi::actionPrepareJumpCharge);
    addChild("JUMPCHARGE_HOLDER", "JUMPCHARGE", ACTION, NULL, (btaction)& CBTSushi::actionJumpCharge);

    addChild("COMBAT_HOLDER", "CHASE", ACTION, (btcondition)& CBTSushi::conditionChase, (btaction)& CBTSushi::actionChase);
    addChild("COMBAT_HOLDER", "IDLE_COMBAT", ACTION, NULL, (btaction)& CBTSushi::actionIdleCombat);


    _footSteps = EngineAudio.playEvent("event:/Enemies/Sushi/Sushi_Footsteps");
    _footSteps.setPaused(true);
}

bool CBTSushi::conditionDeathAnimation() {
  return life <= 0.f && death_animation_started;
}

int CBTSushi::actionDeathStay() {

  return STAY;
}

void CBTSushi::updateBT() {

    if (!h_player.isValid()) {
        h_player = GameController.getPlayerHandle();
    }

    randomTimer -= dt;
    chargeCooldownTimer -= dt;
    jumpChargeCooldownTimer -= dt;
    reevaluatePathTimer -= dt;
    meleeTimer -= dt;
    damageStunTimer -= dt;

		if (resteBlockPorbabiliy <= 0) {
			blockProbability = 40;
			resteBlockPorbabiliy = resteBlockPorbabiliyTimer;
		}else{
			resteBlockPorbabiliy -= dt;
		}
		

    TCompTransform* c_trans = get<TCompTransform>();
    _footSteps.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
    if (nextNavMeshPoint != VEC3().Zero && use_navmesh) { //update path point
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
            CEntity* e_player = (CEntity*)h_player;
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
int CBTSushi::actionNextWaypoint() {
    previousState = currentState;
    currentState = States::NextWaypoint;
    if (_curve == nullptr) {
        if (initialExecution) {
            initialExecution = false;
            TCompTransform* c_trans = get<TCompTransform>();
            VEC3 position = c_trans->getPosition();
            positions.push_back(position); //estos valores quiza deberian ser por parametro tmb

//wtp
/*
            10.4199 24,75 20.9912"
            15.4199 24.75 23.9912"
            12.4199 24.75 25.9912"
            10.4199 24.75 20.9912"
positions.push_back(VEC3(position.x + 5.0f, position.y + 0.0f, position.z + 5.0f)); //estos valores quiza deberian ser por parametro tmb
positions.push_back(VEC3(position.x + 0.0f, position.y + 0.0f, position.z + 0.0f));
positions.push_back(VEC3(position.x + 5.0f, position.y + 0.0f, position.z - 5.0f));
positions.push_back(VEC3(position.x - 5.0f, position.y + 0.0f, position.z + 5.0f));
nextPoint = positions[wtpIndex];
*/
        }
        //wtpIndex = (wtpIndex + 1) % positions.size();
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
            //wtpIndex = (wtpIndex + 1) % _knots.size();
            //nextPoint = _knots[wtpIndex];
//nextPoint = _knots[wtpIndex];

        }
        if (ratio >= 1.0f || ratio < 0.0f) {
            mTravelTime = -mTravelTime;
        }
        ratio += dt * mTravelTime;
        nextPoint = _curve->evaluate(ratio);


        /*VERSION ANTIGUA
        wtpIndex = (wtpIndex + 1) % _knots.size();
        nextPoint = _knots[wtpIndex];
        */

    }
    return LEAVE;
}

void CBTSushi::setCurve(const CCurve* curve) {

    this->_curve = curve; // TO TEST
    _knots = _curve->_knots;
    this->pathCurve = curve->getName();
    /*
        _knots = curve->_knots;

    positions.clear();
    for (int i=0; i < _knots.size();i++) {
        positions.push_back(_knots[i]);
    }
    */
}

string CBTSushi::getNameCurve() {
    return pathCurve;
}



void CBTSushi::setHeightRange(float height) {
    this->height_range = height;
}
void CBTSushi::setViewDistance(float value) {
	this->viewDistance = value;
}

void CBTSushi::setHalfCone(float halfCone) {
	this->half_cone = halfCone;
}


int CBTSushi::actionSeekWaypoint() {
    previousState = currentState;
    currentState = States::SeekWaypoint;
    if (conditionPlayerInView() || inCombat || conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
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
        if (_footSteps.getPaused()) {
            _footSteps.setPaused(false);
        }
        //------------------------------- navmesh code

        if (use_navmesh) {
            //if (reevaluatePathTimer <= 0) {
            reevaluatePathTimer = reevaluatePathDelay;

            TCompTransform* c_trans = get<TCompTransform>();
            VEC3 position = c_trans->getPosition();
            //wtp 
            generateNavmesh(position, nextPoint, false);

            if (navmeshPath.size() > 0) {
                navMeshIndex = 0;
                nextNavMeshPoint = navmeshPath[navMeshIndex];
            }

            //}

            c_trans->rotateTowards(nextNavMeshPoint, rotationSpeed, dt);
        }
        else {
            c_trans->rotateTowards(nextPoint, rotationSpeed, dt);
        }

        //------------------------------- end navmesh code
        return STAY;
    }
}

int CBTSushi::actionIdleCombat() {
    previousState = currentState;
    currentState = States::IdleCombat;
    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    c_trans->rotateTowards(p_trans->getPosition());
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::IDLE_LOOP, 1.f);
    _footSteps.setPaused(true);
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
    return LEAVE;
}

int CBTSushi::actionSalute() {
    previousState = currentState;
    currentState = States::Salute;
    hasSaluted = true;
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    _footSteps.setPaused(true);
    TCompName* cname = get<TCompName>();
    if (saluteElapsed < saluteDuration) {
        //dbg("%s executes Salute\n", cname->getName());
        TCompTransform* c_trans = get<TCompTransform>();
        CEntity* e_player = (CEntity*)h_player;
        TCompTransform* p_trans = e_player->get<TCompTransform>();
        //LookAt_Player(c_trans, player_position);
        //c_trans->rotateTowards(p_trans->getPosition());
        //Salute code
        saluteElapsed += dt;
        return STAY;
    }
    else {
        dbg("%s leaves SALUTE\n", cname->getName());
        //ChangeState("CHASE");
        return LEAVE;
    }
}

int CBTSushi::actionPrepareJumpCharge() {
    previousState = currentState;
    currentState = States::PrepareJumpCharge;
    hasBlocked = false;
    if (conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
    _footSteps.setPaused(true);
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    TCompName* cname = get<TCompName>();
    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    //Rotation Control
    //LookAt_Player(c_trans, playerTrans);
    c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
    //End Rotation Control
    //Get desired point

    TCompRigidBody* c_rb = get<TCompRigidBody>();
    VEC3 impulse = c_rb->getImpulse();
    if (isGrounded()) {
        jumpPosition = c_trans->getPosition();
        c_rb->jump(VEC3(0, 11.f, 0));
        //Start animation
        sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_START, 1.f);
        sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_LOOP, 1.f);
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Sushi_Jump");
        audio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
        return STAY;
    }

    if (c_trans->getPosition().y >= (jumpPosition.y + 3.0f)) {
        dbg("%s activated JUMPCHARGE from PREPAREJUMPCHARGE\n", cname->getName());
        VEC3 height = c_trans->getPosition();
        //ChangeState("JUMPCHARGE");
        jumpPosition = VEC3().Zero;
        chargeObjective = p_trans->getPosition();
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_ReadyWeapon");
        audio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
        return LEAVE;
    }
    else {
        return STAY;
    }
}

int CBTSushi::actionJumpCharge() {
    previousState = currentState;
    currentState = States::JumpCharge;
    TCompRigidBody* c_rb = get<TCompRigidBody>();
    _footSteps.setPaused(true);
    if (conditionGravityReceived() || conditionImpactReceived() || collided || conditionFear()) {
        c_rb->enableGravity(true);
        collided = false;
        return LEAVE;
    }
    _chargeAudio.stop();
    TCompTransform* c_trans = get<TCompTransform>();
    if (!_jumpChargeAudioPlaying) {
        _jumpChargeAudio = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_Charge");
        _jumpChargeAudio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
        dbg("starting jumpcharge audio\n");
        _jumpChargeAudioPlaying = true;
    }
    else {
        _jumpChargeAudio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
    }
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::JUMPCHARGE_LOOP, 1.f);

    c_rb->enableGravity(false);
    TCompName* cname = get<TCompName>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* player_position = e_player->get<TCompTransform>();
    //Move towards that direction
    VEC3 dir = (chargeObjective - c_trans->getPosition()) * jumpChargeSpeed;
    dir *= dt;
    TCompCollider* c_cc = get<TCompCollider>();
    if (c_cc) {
        c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
    }
    //End Charge

    c_trans->rotateTowards(chargeObjective);
    //Exit condition
    if (Vector3::Distance(chargeObjective, c_trans->getPosition()) < distanceCheckThreshold) {
        dbg("%s activated CHASE from JUMPCHARGE\n", cname->getName());
        //ChangeState("CHASE");
        chargeObjective = Vector3().Zero;
        c_rb->enableGravity(true);

        TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
        sushiAnimator->playAnimation(TCompSushiAnimator::JUMPCHARGE_END, 1.f);

        TMsgDamage msg;
        // Who sent this bullet
        msg.h_sender = h_sender;
        msg.h_bullet = CHandle(this).getOwner();
        msg.intensityDamage = explosionDamage;
        msg.senderType = ENEMIES;
        msg.targetType = PLAYER;
        msg.impactForce = impactForceAttack;

        GameController.generateDamageSphere(c_trans->getPosition(), explosionRadius, msg, "player");
        CHandle c = GameController.spawnPrefab("data/prefabs/props/explosion_soja.json", c_trans->getPosition(), QUAT().Identity, 5.0f);
        CEntity* e_sphere = c;

        TCompBuffers* c_buff = e_sphere->get<TCompBuffers>();
        if (c_buff) {
          auto buf = c_buff->getCteByName("TCtesParticles");
          CCteBuffer<TCtesParticles>* data = dynamic_cast<CCteBuffer<TCtesParticles>*>(buf);
          data->emitter_center = c_trans->getPosition();
          data->updateGPU();
        }
        FluidDecalGenerator.generateSingleFluidUncapped(5.f, c_trans->getPosition());
        _jumpChargeAudio.stop();
        _jumpChargeAudioPlaying = false;
        dbg("stopping jumpcharge audio\n");
        return LEAVE;
    }
    else {
        return STAY;
    }
}

int CBTSushi::actionPrepareCharge() {
    previousState = currentState;
    currentState = States::PrepareCharge;
    hasBlocked = false;
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
    _footSteps.setPaused(true);
    TCompTransform* c_trans = get<TCompTransform>();
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    if (!sushiAnimator->isPlaying(TCompSushiAnimator::WAKEUP)) {
        sushiAnimator->playAnimation(TCompSushiAnimator::WAKEUP, 1.f);
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_ReadyWeapon");
        audio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
    }
    //PREPARING CHARGE
    chargeTimer -= dt;


    CEntity* e_player = (CEntity*)h_player;

    TCompTransform* p_trans = e_player->get<TCompTransform>();
    //Rotation Control
    //LookAt_Player(c_trans, playerTrans);
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
        chargePoint = c_trans->getPosition() + (c_trans->getFront() * chargeSpeed * chargeDuration);
        TCompName* cname = get<TCompName>();
        dbg("%s activated CHARGE from PREPARECHARGE\n", cname->getName());
        //ChangeState("CHARGE");
        chargeTimer = chargeDelay;
        return LEAVE;
    }
    else {
        return STAY;
    }
}

int CBTSushi::actionCharge() {

    bool stopCharge = false;
    previousState = currentState;
    currentState = States::Charge;
    TCompRigidBody* c_rb = get<TCompRigidBody>();
    _footSteps.setPaused(true);
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        c_rb->enableGravity(true);
        return LEAVE;
    }
    _jumpChargeAudio.stop();
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::JUMPCHARGE_LOOP, 1.f);
    TCompTransform* c_trans = get<TCompTransform>();
    if (!_chargeAudioPlaying) {
        _chargeAudio = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_Charge");
        _chargeAudio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
        dbg("starting charge audio\n");
        _chargeAudioPlaying = true;
    }
    else {
        _chargeAudio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
    }

    c_rb->enableGravity(false);
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* player_position = e_player->get<TCompTransform>();

    //Move towards that direction
    VEC3 dir = chargeObjective * chargeSpeed * dt;
    impulse = dir;
    TCompCollider* c_cc = get<TCompCollider>();
    if (c_cc) {
        c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
    }
    //TODO: HACER UN RAYCAST PARA DELANTE Y COMPROBAR SI CHOCA CON ALGO
    //raycast para no caer al vacio
    //Si se para antes de colisionar, nunca har� da�o al jugador con esto, lo comento
    //VEC3 pos = c_trans->getPosition();//se lanza el raycast desde la posicion del sushi en la direccion que esta mirando
    //PxF32 attackHeight = c_cc->controller->getHeight() / 2;
    //pos.y = c_trans->getPosition().y + (float)attackHeight;
    //VEC3 direction = c_trans->getFront();
    //VEC3 aux1 = c_trans->getLeft();
    //VEC3 aux2 = c_trans->getUp();
    //direction.Normalize();
    ////VEC3 source = pos;

    //auto scene = EnginePhysics.getScene();
    //PxQueryFilterData filter_data = PxQueryFilterData();
    //filter_data.data.word0 = EnginePhysics.All;//no utilizamos ningun filtro

    //// [in] Define what parts of PxRaycastHit we're interested in
    //const PxHitFlags outputFlags =
    //    PxHitFlag::eDISTANCE
    //    | PxHitFlag::ePOSITION
    //    | PxHitFlag::eNORMAL
    //    ;

    //PxRaycastBuffer hit;
    //PxRaycastHit hitBuffer[10];
    //hit = PxRaycastBuffer(hitBuffer, 10);
    //PxReal _maxDistance = 0.5f; //TEST: este valor habra que modificarlo
    //bool colDetected = scene->raycast(
    //    VEC3_TO_PXVEC3(pos),
    //    VEC3_TO_PXVEC3(direction),
    //    _maxDistance,
    //    hit,
    //    outputFlags,
    //    filter_data
    //);

    //if (colDetected) {
    //    //entonces sigue
    //}
    //else {
    //    //para la carga
    //    stopCharge = true;
    //}

    ////end raycast

    chargeElapsed += dt;
    if (chargeElapsed >= chargeDuration || collided/* || stopCharge*/) {//TODO: AQUI ADD UNA CONDICION QUE SEA QUE VA A CAER Y PARAR A TIEMPO
        //ChangeState("CHASE");
        c_rb->enableGravity(true);
        sushiAnimator->playAnimation(TCompSushiAnimator::JUMPCHARGE_END, 1.f);
        sushiAnimator->playAnimation(TCompSushiAnimator::IDLE_LOOP, 1.f);
        chargeElapsed = 0.f;
        collided = false;


        //------------------------------------ Blackboard

        CEntity* e_player = (CEntity*)h_player;
        TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
        slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);

        if (slotsAvailable) {
            slotsAvailable = false;
            c_bb = e_player->get<TCompBlackboard>();
            c_bb->forgetPlayer(CHandle(this).getOwner(), SUSHI);
        }


        _chargeAudio.stop();
        _chargeAudioPlaying = false;
        //------------------------------------

        dbg("stopping charge audio\n");
        return LEAVE;
    }
    else {
        return STAY;
    }
}

void CBTSushi::generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc)
{
    navmeshPath = EngineNavmesh.findPath(initPos, destPos);
    navmeshPathPoint = 0;
    recalculateNavmesh = recalc;

}

int CBTSushi::actionChase() {

    previousState = currentState;
    currentState = States::Chase;
    TCompName* cname = get<TCompName>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    TCompTransform* c_trans = get<TCompTransform>();
    _jumpChargeAudio.stop();
    _chargeAudio.stop();


    //Rotation Control
    //LookAt_Player(c_trans, player_position);

    //------------------------------- navmesh code

    if (use_navmesh) {
        //NO HACE FALTA EL REVALUATE ESTE 
        /*if (reevaluatePathTimer <= 0) {
            reevaluatePathTimer = reevaluatePathDelay;

            TCompTransform* c_trans = get<TCompTransform>();
            VEC3 position = c_trans->getPosition();
            //wtp
            generateNavmesh(position, p_trans->getPosition(), false);

            if (navmeshPath.size() > 0) {
                navMeshIndex = 0;
                nextNavMeshPoint = navmeshPath[navMeshIndex];
            }

        }*/
        TCompTransform* c_trans = get<TCompTransform>();
        VEC3 position = c_trans->getPosition();
        //wtp 
        generateNavmesh(position, p_trans->getPosition(), false);

        if (navmeshPath.size() > 0 && hayCamino) {
            navMeshIndex = 0;
            nextNavMeshPoint = navmeshPath[navMeshIndex];

            //----ESTO ESTABA FUERA DEL IF y no habia ELSE
            Vector3 dir = Vector3();
            dir = c_trans->getFront() * chaseSpeed * dt;
            TCompCollider* c_cc = get<TCompCollider>();

            TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
            sushiAnimator->playAnimation(TCompSushiAnimator::WALK_LOOP, 1.5f);
            if (c_cc)
                c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
            //End Movement Control
            VEC3 m_hitPos = VEC3();
            //TCompTransform* c_trans = get<TCompTransform>();
            VEC3 currentPosition = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
            VEC3 frontOffset = VEC3(currentPosition.x, currentPosition.y, currentPosition.z + 1.5);
            bool isIntersectionWithNavmesh = EngineNavmesh.raycast(currentPosition, frontOffset, m_hitPos);
            if (isIntersectionWithNavmesh) {
                c_trans->rotateTowards(nextNavMeshPoint, 600, dt);
            }
            else {
                c_trans->rotateTowards(nextNavMeshPoint, rotationSpeed, dt);
            }
            //-----
        }
    }
    else {
        //Movement Control
        TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
        sushiAnimator->playAnimation(TCompSushiAnimator::WALK_LOOP, 1.5f);
        Vector3 dir = Vector3();
        dir = c_trans->getFront() * chaseSpeed * dt;
        TCompCollider* c_cc = get<TCompCollider>();
        if (c_cc)
            c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
        //End Movement Control

        c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
    }

    //------------------------------- end navmesh code
    if (_footSteps.getPaused()) {
        _footSteps.setPaused(false);
    }

    //End Rotation Control
    return LEAVE;
}

int CBTSushi::actionBlock() {
    previousState = currentState;
    currentState = States::Block;
    isBlocking = true;
    if (conditionOnAir() || conditionGravityReceived() || conditionFear() || damaged) {
        return LEAVE;
    }
    if (blockRemaining > 0 && isBlocking) {
        blockRemaining -= dt;
        TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
        sushiAnimator->playAnimation(TCompSushiAnimator::BLOCK_LOOP, 1.f);
        _footSteps.setPaused(true);
        _jumpChargeAudio.stop();
        _chargeAudio.stop();

        return STAY;
    }
    else {
        isBlocking = false;
        hasBlocked = true;
        blockRemaining = blockDuration;
        //ChangeState("IDLEWAR");
        TCompName* cname = get<TCompName>();
        dbg("%s finished BLOCKING and went back to IDLEWAR \n", cname->getName());
        return LEAVE;
    }
}

int CBTSushi::actionBlockOrbitRight() {
    previousState = currentState;
    currentState = States::BlockOrbitRight;
    isBlocking = true;
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    if (blockRemaining > 0 && isBlocking) {
        CEntity* e_player = (CEntity*)h_player;
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
        //LookAt_Player(c_trans, player_position);
        c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
        //End Rotation Control		

        //tranlacion de -EP
        VEC3 dir = e_pos - c_trans->getPosition();
        dir.Normalize();
        dir = dir * orbitSpeed;
        dir *= dt;

        TCompCollider* c_cc = get<TCompCollider>();
        if (c_cc) {
            c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
        }

        blockRemaining -= dt;
        return STAY;
    }
    else {
        isBlocking = false;
        hasBlocked = true;
        blockRemaining = blockDuration;
        TCompName* cname = get<TCompName>();
        dbg("%s leaves BLOCK_ORBIT_RIGHT\n", cname->getName());
        //ChangeState("IDLEWAR");
        return LEAVE;
    }
}

int CBTSushi::actionBlockOrbitLeft() {
    previousState = currentState;
    currentState = States::BlockOrbitLeft;
    isBlocking = true;
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    if (blockRemaining > 0 && isBlocking) {
        CEntity* e_player = (CEntity*)h_player;
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
        //LookAt_Player(c_trans, player_position);
        c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
        //End Rotation Control		

        VEC3 dir = e_pos - c_trans->getPosition();
        dir.Normalize();
        dir = dir * orbitSpeed;
        dir *= dt;

        TCompCollider* c_cc = get<TCompCollider>();
        if (c_cc) {
            c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
        }

        blockRemaining -= dt;
        return STAY;
    }
    else {
        isBlocking = false;
        hasBlocked = true;
        blockRemaining = blockDuration;
        TCompName* cname = get<TCompName>();
        dbg("%s leaves BLOCK_ORBIT_LEFT\n", cname->getName());
        //ChangeState("IDLEWAR");
        return LEAVE;
    }
}

int CBTSushi::actionMelee1() {
    previousState = currentState;
    currentState = States::Melee1;
    hasBlocked = false;
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
    _audioPlaying.stop();
    _footSteps.setPaused(true);
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::IDLE_LOOP, 1.f);
    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    //Rotation Control
    //LookAt_Player(c_trans, player_position);
    c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
    //End Rotation Control
    if (meleeTimer <= 0) {
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_Attack");
        audio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
        sushiAnimator->playAnimation(TCompSushiAnimator::ATTACK1, 1.2f);

        TCompName* cname = get<TCompName>();
        TCompCollider* comp_collider = get<TCompCollider>();
        //Create a collider sphere where we want to detect collision
        PxSphereGeometry geometry(1.0); //NO HARDCODEADO

        Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
        PxF32 attackHeight = comp_collider->controller->getHeight() / 2;
        damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;

        physx::PxFilterData pxFilterData;
        pxFilterData.word0 = EnginePhysics.Player | EnginePhysics.Product;
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
                        Send_DamageMessage<TMsgDamage>(entityContact, melee1Damage);
                        //Check for MELEE2
                        meleeTimer = meleeDelay;
                        melee1Executed = true;
                    }
                }
            }
        }
        return LEAVE;
    }
    else {
        return STAY;
    }
}

int CBTSushi::actionMelee2() {
    previousState = currentState;
    currentState = States::Melee2;
    hasBlocked = false;
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
    _audioPlaying.stop();
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::IDLE_LOOP, 1.f);
    _footSteps.setPaused(true);

    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    //Rotation Control
    //LookAt_Player(c_trans, player_position);
    c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
    //End Rotation Control
    if (meleeTimer <= 0) {
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_Attack");
        audio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
        sushiAnimator->playAnimation(TCompSushiAnimator::ATTACK2, 1.2f);

        TCompName* cname = get<TCompName>();
        TCompCollider* comp_collider = get<TCompCollider>();
        //Create a collider sphere where we want to detect collision
        PxSphereGeometry geometry(1.0); //NO HARDCODEADO

        Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
        PxF32 attackHeight = comp_collider->controller->getHeight() / 2;
        damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;

        physx::PxFilterData pxFilterData;
        pxFilterData.word0 = EnginePhysics.Player | EnginePhysics.Product;
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
                        Send_DamageMessage<TMsgDamage>(entityContact, melee2Damage);
                        meleeTimer = meleeDelay;
                        melee2Executed = true;
                    }
                }
            }
        }
        return LEAVE;
    }
    else {
        return STAY;
    }
}

int CBTSushi::actionMelee3() {
    previousState = currentState;
    currentState = States::Melee3;
    hasBlocked = false;
    if (conditionOnAir() || conditionGravityReceived() || conditionImpactReceived() || conditionFear()) {
        return LEAVE;
    }
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
    _audioPlaying.stop();
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::IDLE_LOOP, 1.f);
    _footSteps.setPaused(true);

    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    //Rotation Control
    //LookAt_Player(c_trans, player_position);
    c_trans->rotateTowards(p_trans->getPosition(), rotationSpeed, dt);
    //End Rotation Control
    if (meleeTimer <= 0) {
        AudioEvent audio = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_Attack");
        audio.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
        sushiAnimator->playAnimation(TCompSushiAnimator::ATTACK3, 1.3f);

        TCompName* cname = get<TCompName>();
        TCompCollider* comp_collider = get<TCompCollider>();
        //Create a collider sphere where we want to detect collision
        PxSphereGeometry geometry(1.0); //NO HARDCODEADO

        Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
        PxF32 attackHeight = comp_collider->controller->getHeight() / 2;
        damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;

        physx::PxFilterData pxFilterData;
        pxFilterData.word0 = EnginePhysics.Player | EnginePhysics.Product;
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
                        Send_DamageMessage<TMsgDamage>(entityContact, melee3Damage);
                        meleeTimer = meleeDelay;
                    }
                }
            }
        }
        return LEAVE;
    }
    else {
        return STAY;
    }
}

int CBTSushi::actionOnAir() {
    previousState = currentState;
    currentState = States::OnAir;
    if (isGrounded() && impulse.y <= 0.0f || isDeadForFallout) {
        impulse.y = 0.0f;
        return LEAVE;
    }

    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_LOOP, 1.f);
    _footSteps.setPaused(true);

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

int CBTSushi::actionImpactReceived() {
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

    return LEAVE;
}

int CBTSushi::actionGravityReceived() {
    previousState = currentState;
    currentState = States::GravityReceived;
    _footSteps.setPaused(true);
    _jumpChargeAudio.stop();
    _chargeAudio.stop();
    _audioPlaying.stop();
    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    inCombat = true;
    combatViewDistance += Vector3::Distance(c_trans->getPosition(), p_trans->getPosition()) + 10.f;
    TCompCollider* c_cc = get<TCompCollider>();
    c_cc->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    if (battery_time > 0) {
        TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
        sushiAnimator->playAnimation(TCompSushiAnimator::JUMP_LOOP, 1.f);

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

int CBTSushi::actionFear() {
    previousState = currentState;
    currentState = States::FearReceived;
    if (_onFireArea) {
        _fearTimer = _fearDuration;
    }
    if (_fearTimer > 0.f) {
        _jumpChargeAudio.stop();
        _chargeAudio.stop();
        _audioPlaying.stop();
        TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
        sushiAnimator->playAnimation(TCompSushiAnimator::WALK_LOOP, 2.f);

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

int CBTSushi::actionDeath() {
    GameController.addEnemiesKilled(EntityType::SUSHI);
    TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
    sushiAnimator->playAnimation(TCompSushiAnimator::BLOCK_BREAK, 1.f);
    sushiAnimator->playAnimation(TCompSushiAnimator::DEAD, 1.f);

    //------------------------------------ Blackboard

    CEntity* e_player = (CEntity*)h_player;
    TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
    slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);

    if (slotsAvailable) {
        slotsAvailable = false;
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
        _jumpChargeAudio.stop();
        _chargeAudio.stop();
        _footSteps.stop();
        _audioPlaying.stop();
		CHandle(this).getOwner().destroy();
		CHandle(this).destroy();
        

	}
	//------ENVIO ME HE MUERTO A COMPONENTE DE TRAMPA DE SUISHIS-----
	/*
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
	}*/
	/*
  TEntityParseContext ctx;
  ctx.root_transform = *c_trans;
  parseScene("data/prefabs/vfx/death_sphere.json", ctx);

  TCompSelfDestroy* c_sd = get<TCompSelfDestroy>();
  c_sd->setDelay(0.25f);
  c_sd->enable();

  death_animation_started = true;*/
   _footSteps.setPaused(true);
  return LEAVE;
}
#pragma endregion
//End Actions

//Conditions
#pragma region Conditions
bool CBTSushi::conditionOnAir() {
    return !isGrounded();
}

bool CBTSushi::conditionChase() {
    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    float distance = 6.0;

    if (slotsAvailable) {
        distance = meleeDistance;
    }
    else {
        //TODO distance 
    }


    return (VEC3::Distance(p_trans->getPosition(), c_trans->getPosition()) > distance);
}

bool CBTSushi::conditionPlayerInView() {
    if (player_dead) {
        inCombat = false;
        return false; //no lo ve si esta muerto
    }

    bool res = false;
    if (isView() && checkHeight()) {//tiene que estar dentro de ambos rangos
        res = true;
    }
    if (use_navmesh) {
        //raycast de personaje hacia abajo
        if (!checkHeight()) {
            inCombat = false;
            return false;
        }
        if (isView()) {
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
    /*if (use_navmesh)
        isPlayerInNavmesh();
        if (navmeshPath.size() == 0) {
            res = false;
            initialExecution = true;
            inCombat = false;
        }
    return res;*/
}

bool CBTSushi::conditionSalute() {
    return ((previousState == States::SeekWaypoint || previousState == States::NextWaypoint) && !hasSaluted && rollDice(saluteProbability));
}

bool CBTSushi::conditionMelee() {

    TCompTransform* c_trans = get<TCompTransform>();
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();


    return (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) <= meleeDistance) && checkBlackboard();
}

bool CBTSushi::conditionJumpCharge() {
    //VEC3 start, VEC3 end, VEC3 &m_hitPos
    /*VEC3 m_hitPos = VEC3();
    TCompTransform* c_trans = get<TCompTransform>();
    VEC3 currentPosition = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
    VEC3 frontOffset = VEC3(currentPosition.x, currentPosition.y, currentPosition.z + 100);
	if (use_navmesh) {
		bool charge = EngineNavmesh.raycast(currentPosition, frontOffset, m_hitPos);
	}*/
    //if (!charge) {
    return rollDiceJumpCharge() && checkBlackboard();
    //}
    return false;
}

bool CBTSushi::conditionCharge() {
    /*VEC3 m_hitPos = VEC3();
    TCompTransform* c_trans = get<TCompTransform>();
    VEC3 currentPosition = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
    VEC3 frontOffset = VEC3(currentPosition.x, currentPosition.y, currentPosition.z + 100);
	if (use_navmesh) {
		bool charge = EngineNavmesh.raycast(currentPosition, frontOffset, m_hitPos);
	}*/
    //if (!charge) {
    return rollDiceCharge() && checkBlackboard();
    //}
    return false;
}

bool CBTSushi::conditionBlock() {

    return rollDice(blockProbability) && !hasBlocked && !isBlocking;
}

bool CBTSushi::conditionBlockOrbit() {
    return rollDice(orbitProbability) && !hasBlocked;
}

bool CBTSushi::conditionCombo3() {
    bool res = (bt_dist(bt_mt) < meleeComboProbability) && melee2Executed;
    melee1Executed = false;
    melee2Executed = false;
    return res;
}

bool CBTSushi::conditionCombo2() {
    return (bt_dist(bt_mt) < meleeComboProbability) && melee1Executed;;
}

bool CBTSushi::conditionImpactReceived() {
    return damageStunTimer > 0.f;
}

bool CBTSushi::conditionFear() {
    return _onFireArea;
}

bool CBTSushi::conditionGravityReceived() {
    return beingAttracted && battery_time > 0;
}

bool CBTSushi::conditionDeath() {
    return life <= 0.f && !death_animation_started;
}
#pragma endregion
//End Conditions


/* --------------------- Helper Functions ---------------------*/
#pragma region Helper Functions

bool CBTSushi::rollDice(int probability) {
    if (randomTimer <= 0) {
        randomTimer = randomDelay;
        int dice = bt_dist(bt_mt);
        return (dice < probability);
    }
    else {
        return false;
    }
}

bool CBTSushi::rollDiceCharge() {
    if (chargeCooldownTimer <= 0) {
        TCompTransform* c_trans = get<TCompTransform>();
        CEntity* e_player = (CEntity*)h_player;
        TCompTransform* p_trans = e_player->get<TCompTransform>();

        chargeCooldownTimer = chargeCooldown;
        int dice = bt_dist(bt_mt);
        return (dice < chargeProbability) && (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) > chargeDistanceThreshold);
    }
    else {
        return false;
    }
}

bool CBTSushi::rollDiceJumpCharge() {
    if (jumpChargeCooldownTimer <= 0) {
        TCompTransform* c_trans = get<TCompTransform>();
        CEntity* e_player = (CEntity*)h_player;
        TCompTransform* p_trans = e_player->get<TCompTransform>();

        jumpChargeCooldownTimer = jumpChargeCooldown;
        int dice = bt_dist(bt_mt);
        return (dice < jumpChargeProbability) && (Vector3::Distance(p_trans->getPosition(), c_trans->getPosition()) > jumpChargeDistanceThreshold);
    }
    else {
        return false;
    }
}

template <typename T>
void CBTSushi::Send_DamageMessage(CEntity* entity, float dmg) {
    //Send damage mesage
    T msg;
    // Who sent this bullet
    msg.h_sender = h_sender;
    msg.h_bullet = CHandle(this).getOwner();
    msg.intensityDamage = dmg;
    msg.senderType = ENEMIES;
    msg.targetType = PLAYER;
    msg.impactForce = impactForceAttack;
    entity->sendMsg(msg);
}

bool CBTSushi::isView() {

    if (!h_player.isValid()) {
        h_player = GameController.getPlayerHandle();
    }
    CEntity* e_player = (CEntity*)h_player;
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
					isPlayerInNavmesh();
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
            if (!sighted) {
                inCombat = false;
                //------------------------------------ Blackboard

                CEntity* e_player = (CEntity*)h_player;
                TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
                slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);

                if (slotsAvailable) {
                    slotsAvailable = false;
                    c_bb->forgetPlayer(CHandle(this).getOwner(), SUSHI);
                }


                //-----------------------------------
            }
            return sighted;
        }
        else {
            bool sighted = (distance <= combatViewDistance);
            if (!sighted) {
                inCombat = false;
                //------------------------------------ Blackboard

                CEntity* e_player = (CEntity*)h_player;
                TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
                slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);

                if (slotsAvailable) {
                    slotsAvailable = false;
                    c_bb->forgetPlayer(CHandle(this).getOwner(), SUSHI);
                }


                //-----------------------------------
            }
            return sighted;
        }
    }
    return false;
}

bool CBTSushi::isGrounded() {
    TCompRigidBody* r_body = get<TCompRigidBody>();
    return r_body->is_grounded;
}

void CBTSushi::OnAir() {
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


void CBTSushi::load(const json& j, TEntityParseContext& ctx) {
    //Pathing
    nWaypoints = j.value("nWaypoints", nWaypoints);
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

	//half_cone = 360;
	//viewDistance = 1000;
	

    //End View Ranges

    //Core Values
    life = j.value("life", life);
    patrolSpeed = j.value("patrolSpeed", patrolSpeed);
    chaseSpeed = j.value("chaseSpeed", chaseSpeed);
    rotationSpeed = j.value("rotationSpeed", rotationSpeed);
    damageStunDuration = j.value("damageStunDuration", damageStunDuration);
    damageStunBackwardsSpeed = j.value("damageStunBackwardsSpeed", damageStunBackwardsSpeed);
    distanceCheckThreshold = j.value("distanceCheckThreshold", distanceCheckThreshold);
    randomDelay = j.value("randomDelay", randomDelay);
    //End Core Values

    //Melee Values
    meleeComboProbability = j.value("meleeComboProbability", meleeComboProbability);
    melee1Damage = j.value("melee1Damage", melee1Damage);
    melee2Damage = j.value("melee2Damage", melee2Damage);
    melee3Damage = j.value("melee3Damage", melee3Damage);
    meleeDistance = j.value("meleeDistance", meleeDistance);
    meleeDelay = j.value("meleeDelay", meleeDelay);
    //End Melee Values

    //Charge Values
    chargeProbability = j.value("chargeProbability", chargeProbability);
    chargeSpeed = j.value("chargeSpeed", chargeSpeed);
    chargeDamage = j.value("chargeDamage", chargeDamage);
    chargeDelay = j.value("chargeDelay", chargeDelay);
    chargeDuration = j.value("chargeDuration", chargeDuration);
    chargeCooldown = j.value("chargeCooldown", chargeCooldown);
    chargeDistanceThreshold = j.value("chargeDistanceThreshold", chargeDistanceThreshold);
    //End Charge Values

    //Jump Charge Values
    jumpChargeProbability = j.value("jumpChargeProbability", jumpChargeProbability);
    jumpSpeed = j.value("jumpSpeed", jumpSpeed);
    jumpHeight = j.value("jumpHeight", jumpHeight);
    jumpChargeSpeed = j.value("jumpChargeSpeed", jumpChargeSpeed);
    jumpChargeDamage = j.value("jumpChargeDamage", jumpChargeDamage);
    jumpChargeCooldown = j.value("jumpChargeCooldown", jumpChargeCooldown);
    jumpChargeDistanceThreshold = j.value("jumpChargeDistanceThreshold", jumpChargeDistanceThreshold);
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

    //Fear Values
    _fearSpeed = j.value("_fearSpeed", _fearSpeed);
    _fearDuration = j.value("orbitSpeed", _fearDuration);
    //End Fear Values

    //navmesh values 
    use_navmesh = j.value("use_navmesh", use_navmesh);
    reevaluatePathDelay = j.value("reevaluatePathDelay", reevaluatePathDelay);
    //end navmesh values


    //define curve
    if (j.count("curve") > 0) {
        pathCurve = j.value("curve", "");
        _curve = Resources.get(j.value("curve", ""))->as<CCurve>();
    }


    this->create("bt_sushi");
}

void CBTSushi::debugInMenu() {
    //Core Values
    //ImGui::TreeNode("Core Values");
    ImGui::DragFloat("life", &life, 0.1f, 0.f, 100.f);
    ImGui::Checkbox("_pausedBT", &_pausedBT);
    ImGui::Text(getState().c_str());
}

void CBTSushi::renderDebug() {
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
        if (currentState == States::Charge) {
            drawCircle(chargePoint, meleeDistance, VEC4(1, 0, 0, 1));
            drawLine(pos, chargePoint, VEC4(1, 0, 0, 1));
        }
        else if (currentState == States::JumpCharge) {
            drawCircle(chargeObjective, meleeDistance, VEC4(1, 0, 0, 1));
            drawLine(pos, chargeObjective, VEC4(1, 0, 0, 1));
        }
        else if (currentState == States::Melee1 || currentState == States::Melee2 || currentState == States::Melee3) {
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

void CBTSushi::registerMsgs() {
    DECL_MSG(CBTSushi, TMsgBlackboard, onBlackboardMsg);
    DECL_MSG(CBTSushi, TMsgFireAreaEnter, onFireAreaEnter);
    DECL_MSG(CBTSushi, TMsgFireAreaExit, onFireAreaExit);
    DECL_MSG(CBTSushi, TMsgDamage, onGenericDamageInfoMsg);
    DECL_MSG(CBTSushi, TMsgOnContact, onCollision);
    DECL_MSG(CBTSushi, TMsgGravity, onGravity);
    DECL_MSG(CBTSushi, TMsgBTPaused, onMsgBTPaused);
	DECL_MSG(CBTSushi, TMSgTriggerFalloutDead, onTriggerFalloutDead);
	DECL_MSG(CBTSushi, TMsgDeleteTrigger, onDeleteTrigger);
	//
	//DECL_MSG(CBTSushi, TMsgDamageToAll, onDamageAll);//Nuevo, esto para el caso de que te caes
}

void CBTSushi::onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg) {
    life -= msg.damage;
    isDeadForFallout = msg.falloutDead;
    if (life < 0) {
        life = 0;
    }
}

void CBTSushi::onDeleteTrigger(const TMsgDeleteTrigger& msg) {
	isDeadForTrigger = true;
	life = 0;
}


/*
void CBTSushi::onDamageAll(const TMsgDamageToAll& msg) {//se recibe este mensaje solo cuando se cae por el collider ese triggerDamage
    life -= msg.intensityDamage;
    if (life < 0) {
        life = 0;
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
        CHandle(this).getOwner().destroy();
        CHandle(this).destroy();
    }
}*/

void CBTSushi::onBlackboardMsg(const TMsgBlackboard& msg) {
    player_dead = msg.player_dead;

    if (player_dead) {
        inCombat = false;
    }
}

void CBTSushi::onGenericDamageInfoMsg(const TMsgDamage& msg) {
    if (isPaused()) {
        return;
    }
    if (msg.targetType & EntityType::SUSHI) {
        TCompTransform* c_trans = get<TCompTransform>();
        TCompSushiAnimator* sushiAnimator = get<TCompSushiAnimator>();
        if (isBlocking && msg.senderType == EntityType::PLAYER && msg.damageType != PowerType::CHARGED_ATTACK) {
            dbg("Damage blocked\n");
            if (!_audioPlaying.isPlaying()) {
                _audioPlaying = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_Parry");
                _audioPlaying.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
            }
            sushiAnimator->playAnimation(TCompSushiAnimator::BLOCK_HIT, 0.7f);
        }
        else {
					if (msg.damageType == PowerType::FIRE) {
						blockProbability = 100;
					}
            h_sender = msg.h_sender;
            damageSource = msg.position;
            life -= msg.intensityDamage;
            direction_to_damage = c_trans->getPosition() - msg.position;
            impactForce = msg.impactForce;
            //direction_to_damage.y = 1.0f;
            direction_to_damage.Normalize();
			if (msg.damageType == PowerType::CHARGED_ATTACK) {
				blockRemaining = 0;
                sushiAnimator->playAnimation(TCompSushiAnimator::BLOCK_BREAK_GET_UP, 1.f);
                if (!_audioPlaying.isPlaying()) {
                    _audioPlaying = EngineAudio.playEvent("event:/Enemies/Sushi/Melee_ParryBreak");
                    _audioPlaying.set3DAttributes(c_trans->getPosition(), c_trans->getFront(), c_trans->getUp());
                }
            }
            else {
                sushiAnimator->playAnimation(TCompSushiAnimator::DAMAGED, 1.f);
            }
						
            TCompRigidBody* c_rbody = get<TCompRigidBody>();
            if (c_rbody) {
                c_rbody->addForce(direction_to_damage * msg.impactForce);
            }
            if (life < 0) {
                life = 0;
            }
            //ChangeState("IMPACT");
            damageStunTimer = damageStunDuration;
            FluidDecalGenerator.generateFluid(msg.impactForce, c_trans->getPosition());
        }
    }
}

void CBTSushi::onCollision(const TMsgOnContact& msg) {
    if (isPaused()) {
        return;
    }
    CEntity* source_of_impact = (CEntity*)msg.source.getOwner();
    if (source_of_impact) {
        TCompCollider* c_tag = source_of_impact->get<TCompCollider>();
        if (c_tag) {
            PxShape* colShape;
            c_tag->actor->getShapes(&colShape, 1, 0);
            PxFilterData col_filter_data = colShape->getSimulationFilterData();

            //If I collide with the player while I'm charging, I send TMsgDamageToPlayer
            if (col_filter_data.word0 & EnginePhysics.Player || col_filter_data.word0 & EnginePhysics.Product) {
                if (currentState == States::Charge) {
                    meleeTimer = meleeDelay;
                    collided = true;
                    Send_DamageMessage<TMsgDamage>(source_of_impact, chargeDamage);
                    //ChangeState("IDLEWAR");
                    return;
                }
                if (currentState == States::JumpCharge) {
                    meleeTimer = meleeDelay;
                    collided = true;
                    Send_DamageMessage<TMsgDamage>(source_of_impact, jumpChargeDamage);
                    //ChangeState("IDLEWAR");
                    return;
                }
                //If I collide with something other than the player, but I'm charging, I stop charging
            }
            else if (col_filter_data.word0 & EnginePhysics.Obstacle && (currentState == States::Charge || currentState == States::JumpCharge)) {
                //collided = true;
            }
            else if (col_filter_data.word0 & EnginePhysics.Enemy && (currentState == States::Charge || currentState == States::JumpCharge)) {
                TMsgDamage msg;
                // Who sent this bullet
                msg.h_sender = CHandle(this).getOwner();
                msg.h_bullet = CHandle(this).getOwner();
                msg.intensityDamage = 0.f;
                msg.senderType = ENEMIES;
                msg.targetType = ENEMIES;
                msg.impactForce = 2.f;
                source_of_impact->sendMsg(msg);
            }
            else if (col_filter_data.word0 & EnginePhysics.Enemy && battery_time > 0) {
                TMsgDamage msg;
                // Who sent this bullet
                msg.h_sender = CHandle(this).getOwner();
                msg.h_bullet = CHandle(this).getOwner();
                msg.intensityDamage = 0.f;
                msg.senderType = ENEMIES;
                msg.targetType = ENEMIES;
                msg.impactForce = 2.f;
                source_of_impact->sendMsg(msg);
            }
        }
    }
}

void CBTSushi::onGravity(const TMsgGravity& msg) {
    if (isPaused()) {
        return;
    }
    if (!beingAttracted && msg.time_effect > 0) {
        h_sender = msg.h_sender;
        h_bullet = msg.h_bullet;
        //obtener posicion
        beingAttracted = true;
        CEntity* e_bullet = (CEntity*)h_bullet;
        TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
        battery_position = msg.position;
        battery_time = msg.time_effect;
        attractionForce = msg.attractionForce;
        distanceToBattery = msg.distance;

        //ChangeState("BATTERYEFFECT");
    }if (beingAttracted && h_bullet == msg.h_bullet) {

        //obtener posicion
        beingAttracted = true;
        CEntity* e_bullet = (CEntity*)h_bullet;
        TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
        battery_position = msg.position;

        //ChangeState("BATTERYEFFECT");
    }
}

void CBTSushi::onFireAreaEnter(const TMsgFireAreaEnter& msg) {
    if (isPaused()) {
        return;
    }
    dbg("Sushi stepped in the fire!\n");
    _onFireArea = true;
    _fearTimer = _fearDuration;
    _fearOrigin = msg.areaCenter;
}
void CBTSushi::onFireAreaExit(const TMsgFireAreaExit& msg) {
    if (isPaused()) {
        return;
    }
    dbg("Sushi got out of the fire\n");
    _onFireArea = false;
}

std::string CBTSushi::getState() {
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
    case States::IdleCombat:
        return "IdleCombat";
        break;
    case States::Salute:
        return "Salute";
        break;
    case States::PrepareJumpCharge:
        return "PrepareJumpCharge";
        break;
    case States::JumpCharge:
        return "JumpCharge";
        break;
    case States::PrepareCharge:
        return "PrepareCharge";
        break;
    case States::Charge:
        return "Charge";
        break;
    case States::Chase:
        return "Chase";
        break;
    case States::Block:
        return "Block";
        break;
    case States::BlockOrbitRight:
        return "BlockOrbitRight";
        break;
    case States::BlockOrbitLeft:
        return "BlockOrbitLeft";
        break;
    case States::Melee1:
        return "Melee1";
        break;
    case States::Melee2:
        return "Melee2";
        break;
    case States::Melee3:
        return "Melee3";
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
    default:
        return "unspecified";
        break;
    }
}

bool CBTSushi::checkHeight() {
    bool res = false;
    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* player_position = e_player->get<TCompTransform>();
    float playerHeight = player_position->getPosition().y;
    TCompTransform* c_trans = get<TCompTransform>();
    float enemyHeight = c_trans->getPosition().y;

    if (height_range > abs(playerHeight - enemyHeight)) {
        res = true;
    }

    return res;
}

bool CBTSushi::checkBlackboard() {
    CEntity* e_player = (CEntity*)h_player;
    TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
    slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), SUSHI);
    return slotsAvailable;
}





bool CBTSushi::isPlayerInNavmesh() {

    CEntity* e_player = (CEntity*)h_player;
    TCompTransform* p_trans = e_player->get<TCompTransform>();
    TCompTransform* c_trans = get<TCompTransform>();
    VEC3 position = c_trans->getPosition();
    //wtp
    VEC3 posPlayer = calculatePositionGround();//Nueva para BUG salto player
    generateNavmesh(position, posPlayer, false);
    //inCombat = false;
    return true;
    /*if (use_navmesh) {
        VEC3 m_hitPos = VEC3();
        TCompTransform* c_trans = get<TCompTransform>();
        VEC3 currentPosition = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
        VEC3 frontOffset = VEC3(currentPosition.x, currentPosition.y, currentPosition.z + 1);
        float isIntersectionWithNavmesh = EngineNavmesh.wallDistance(currentPosition);
        dbg("INTER:%f\n",isIntersectionWithNavmesh);
        if (navmeshPath.size() == 0 &&  (isIntersectionWithNavmesh < 0.1f) ) {
            dbg("Entro en no esta el player en Navmesh\n");
            return false;
        }
        else {
            return true;
        }
    }
    return true;*/
}

VEC3 CBTSushi::calculatePositionGround() {
    CEntity* e_player = (CEntity*)h_player;
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
    PxQueryFilterData filter_data = PxQueryFilterData();
    filter_data.data.word0 = EnginePhysics.NotPlayer;

    PxVec3 origin = VEC3_TO_PXVEC3(char_pos);
    PxVec3 unitDir = VEC3_TO_PXVEC3((-c_trans->getUp()));//direccion abajo
    bool res = EnginePhysics.gScene->raycast(origin, unitDir, maxDistance, hit, outputFlags, filter_data);
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
                        if (candidate != nullptr) {
                            //dbg("el candidato obj es valido nombre = %s  \n", candidate->getName());
                        }
                        positionJump = PXVEC3_TO_VEC3(hit.getAnyHit(closestIdx).position);
                    }
                }
            }
        }
    }
    /*TCompCharacterController* characterController = e_player->get<TCompCharacterController>();
    if (!(characterController->is_grounded) ) {
        dbg("RAYCAST PLAYER: positionJump.x:%f,positionJump.y:%fpositionJump.z:%f\n", positionJump.x, positionJump.y, positionJump.z);
        VEC3 position = c_trans->getPosition();
        dbg("POS SUSHY: position.x:%f,position.y:%fposition.z:%f\n", position.x, position.y, position.z);
        dbg("------------------------------------------\n");
    }*/
    return positionJump;
}