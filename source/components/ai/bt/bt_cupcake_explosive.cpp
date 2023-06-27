#include "mcv_platform.h"
#include "engine.h"
#include "entity/entity.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/john/comp_bullet.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_tags.h"
#include "time.h"
#include "stdlib.h"
#include "modules/module_physics.h"
#include "modules/game/module_fluid_decal_generator.h"
#include "bt_cupcake_explosive.h"
#include "components/ai/others/comp_blackboard.h"
#include "random"

using namespace physx;
std::mt19937 bt_cup_expl(1942);
std::uniform_int_distribution<int> bt_range_expl(0, 100);

DECL_OBJ_MANAGER("bt_cupcake_explosive", CBTCupcake_explosive);
void CBTCupcake_explosive::create(string s)//crear el arbol
{
	name = s; //createRoot y addChild debe hacerse en orden, sino peta -- nunca definir un huerfano directamente 
	// padre - hijo - tipo - condition - action

	if (_initiallyPaused) {
		setPaused(true);
	}

	createRoot("CUPCAKE_EXPLOSIVE", PRIORITY, NULL, NULL);

	//pasive states:

	addChild("CUPCAKE_EXPLOSIVE", "ON_EXPLODE", ACTION, (btcondition)&CBTCupcake_explosive::conditionExplode, (btaction)&CBTCupcake_explosive::actionExplode);
	addChild("CUPCAKE_EXPLOSIVE", "ON_DEATH", ACTION, (btcondition)&CBTCupcake_explosive::conditionDeath, (btaction)&CBTCupcake_explosive::actionDeath);
	addChild("CUPCAKE_EXPLOSIVE", "ON_GRAVITY", ACTION, (btcondition)&CBTCupcake_explosive::conditionGravityReceived, (btaction)&CBTCupcake_explosive::actionGravityReceived);
	addChild("CUPCAKE_EXPLOSIVE", "ON_IMPACT", ACTION, (btcondition)&CBTCupcake_explosive::conditionImpactReceived, (btaction)&CBTCupcake_explosive::actionImpactReceived);
	addChild("CUPCAKE_EXPLOSIVE", "ON_RECOIL", ACTION, (btcondition)&CBTCupcake_explosive::conditionRecoilReceived, (btaction)&CBTCupcake_explosive::actionRecoilReceived);

	//active states
	addChild("CUPCAKE_EXPLOSIVE", "INSIGHT", PRIORITY, (btcondition)&CBTCupcake_explosive::conditionView, NULL); //es un priority con una condicion random
	addChild("CUPCAKE_EXPLOSIVE", "CHANGEWPT", ACTION, (btcondition)&CBTCupcake_explosive::conditionWptClose, (btaction)&CBTCupcake_explosive::actionChangeWpt);
	addChild("CUPCAKE_EXPLOSIVE", "SEEKWPT", ACTION, NULL, (btaction)&CBTCupcake_explosive::actionSeekWpt);

	//crear un nodo aqui que sea cambiar de estado de salto a oculto cada x tiempo y que el tiempo sea la condicion de entrada
	addChild("INSIGHT", "ATTACK", ACTION, NULL, (btaction)&CBTCupcake_explosive::actionAttack); //falta el action que no es mas que el chase



	if (!h_player.isValid()) {
		h_player = GameController.getPlayerHandle();
	}

}

//--------------ACTIONS---------------
int CBTCupcake_explosive::actionChangeWpt() {
	//accion de cambiar de wpt
  //dbg("cambiando de waypoint\n");

	if (ratio >= 1.0f || ratio < 0.0f) {
		mTravelTime = -mTravelTime;
	}
	ratio += dt * mTravelTime;
	nextPoint = _curve->evaluate(ratio);

	/*wtpIndex = (wtpIndex + 1) % positions.size();
	nextPoint = positions[wtpIndex];*/
	attacking = false;
	return LEAVE;
}

int CBTCupcake_explosive::actionChangeState() {
	evaluateState = evaluateStateTime;

	//random sobre 100
	if (bt_range_expl(bt_cup_expl) < changeStateProb) {
		if (jumpingCupcake) {
			jumpingCupcake = false;
			jump = false;
			//TCompRender* crender = get<TCompRender>();
			//crender->showMeshesWithState(1);

		}
		else {
			jumpingCupcake = true;
			jump = true;
			//TCompRender* crender = get<TCompRender>();
			//crender->showMeshesWithState(0);
		}
	}

	return LEAVE;
}

int CBTCupcake_explosive::actionSeekWpt() {
	//MOVE
  //dbg("siguiendo waypoint -- orbittin value = %i\n", orbitting);
	attacking = false;


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

		movement(nextNavMeshPoint, true);
	}
	else {
		movement(nextPoint, true); 
	}

	return LEAVE;
}

int CBTCupcake_explosive::actionAttack() {
	//Accion de ATACAR/CHASE AL PLAYER IF ESTA MUY LEJOS LEAVE
	//dbg("atacando \n");
	attacking = true;
	jump = true;
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	evaluateState -= dt;
	evaluateAttack -= dt;
	//dbg("el valor de evaluate State es: %f  y jumping cupcake es %i\n", evaluateState, jumpingCupcake);
	//MOVE
	//navmesh
	if (use_navmesh) {
		if (reevaluatePathTimer <= 0) {
			reevaluatePathTimer = reevaluatePathDelay;

			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 position = c_trans->getPosition();
			//wtp 
			generateNavmesh(position, player_position->getPosition(), false);

			if (navmeshPath.size() > 0) {
				navMeshIndex = 0;
				nextNavMeshPoint = navmeshPath[navMeshIndex];
			}

		}

		movement(nextNavMeshPoint, true); 
	}
	else {
		movement(player_position->getPosition(), true); //con esto a true nunca se pondra en hide mientras ataca
	}
	


	return LEAVE;
}


int CBTCupcake_explosive::actionDeath() {
	if (_mySpawner.isValid()) {
		TMsgSpawnerCheckout checkout;
		checkout.enemyHandle = CHandle(this).getOwner();
		((CEntity*)_mySpawner)->sendMsg(checkout);
	}
	GameController.addEnemiesKilled(EntityType::CUPCAKE);

	//------------------------------------ Blackboard
	if (inBlackboard) {
		inBlackboard = false;
		slotsAvailable = false;
		CEntity* e_player = (CEntity *)h_player;
		TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
		c_bb->forgetPlayer(CHandle(this).getOwner(), CUPCAKE);
	}
	//------------------------------------
  TCompTransform* c_trans = get<TCompTransform>();
  if (!isDeadForFallout) {
	  GameController.spawnPuddle(c_trans->getPosition(), c_trans->getRotation(), 0.3f);
  }
  EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Death3D");
  voice.stop();
	CHandle(this).getOwner().destroy();
	CHandle(this).destroy();
	return LEAVE;
}

int CBTCupcake_explosive::actionExplode() {

	//TODO: DELAY Y QUE EXPLOTE CUANDO SEA
	//explosionActive = true;
	
	if (explosionTimer <= 0) { //si se acaba el tiempo
		

		//------------EXPLOSION
		life = 0;
		has_exploded = true;


		TMsgDamage msg;
		// Who sent this bullet
		msg.h_sender = h_sender;
		msg.h_bullet = CHandle(this).getOwner();
		msg.intensityDamage = explosion_damage;
		msg.senderType = ENEMIES;
		msg.targetType = PLAYER;
		msg.impactForce = impactForceAttack;

		//TODO: animacion del cupcake creciendo para explotar 

		TCompTransform* c_trans = get<TCompTransform>();
		GameController.generateDamageSphere(c_trans->getPosition(), explosionRadius, msg, "player");
		GameController.spawnPrefab("data/prefabs/props/explosion_soja.json", c_trans->getPosition(), c_trans->getRotation(), explosionRadius);
		//------------EXPLOSION

		return LEAVE;
	}
	else {
		explosionTimer -= dt;
	}

	return STAY; //DE AQUI YA NO SALE HASTA QUE EXPLOTE
}

int CBTCupcake_explosive::actionGravityReceived() { //TODO : COMPROBAR QUE ESTO FUNCIONE CUANDO LA BATERIA ESTE ARREGLADA :D
	/*
  TCompTransform* c_trans = get<TCompTransform>();
  CEntity* e_player = (CEntity *)h_player;
  TCompTransform* p_trans = e_player->get<TCompTransform>();
  if (battery_time > 0) {
	battery_time -= dt;
	//moverme hacia battery_position y si estoy en batery position no moverme
	float distancia = VEC3::Distance(battery_position, c_trans->getPosition());
	VEC3 dir = VEC3();
	dir = battery_position - c_trans->getPosition();
	dir.Normalize();
	dir *= 20.f; // attractionForce; --> pillar esto de la pila??
	dir *= dt;
	//switch between attraction and repulsion, a few frames each
	TCompCollider* c_cc = get<TCompCollider>();
	if (distancia < distanceToBattery) {
	  //Repulsion
	  if (c_cc) {
		c_cc->controller->move(VEC3_TO_PXVEC3(-dir), 0.0f, dt, PxControllerFilters());
	  }
	}
	else {
	  //Attraction
	  if (c_cc) {
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	  }

	}

	return STAY;
  }
  else {
	beingAttracted = false;
	impacted = false;
	return LEAVE;
  }
  */
	if (life <= 0) {
		return LEAVE;
	}

	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* p_trans = e_player->get<TCompTransform>();
	//inCombat = true;
	//combatViewDistance += Vector3::Distance(c_trans->getPosition(), p_trans->getPosition()) + 10.f;
	TCompCollider* c_cc = get<TCompCollider>();
	c_cc->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	if (battery_time > 0) {
		//moverme hacia battery_position y si estoy en batery position no moverme
		VEC3 repulsionForce = VEC3().Zero;
		VEC3 attractionForce = VEC3().Zero;
		std::uniform_real_distribution<float> float_dist(-1.0, 1.0);
		//VEC3 randomForce = VEC3(0.0f, float_dist(bt_mt), 0.0f);
		//randomForce *= 65.f;
		VEC3 rotationForce = (Vector3().Up).Cross(c_trans->getPosition() - battery_position);
		rotationForce *= 4.f;


		float distance = VEC3::Distance(battery_position, c_trans->getPosition());
		//dbg("Timer is %f\n", repulsionTimer);
		//dbg("Distance is %f\n", distance);
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
			//dbg("Repelling with force %f, %f, %f\n", repulsionForce.x, repulsionForce.y, repulsionForce.z);
		}
		else if (distance > distanceToBattery && repulsionTimer <= 0) {
			//Attraction
			attractionForce = battery_position - c_trans->getPosition();
			attractionForce.Normalize();
			float inverseDistance = 1 / distance;
			attractionForce *= 10.f;
			//dbg("Attracting with force %f, %f, %f\n", attractionForce.x, attractionForce.y, attractionForce.z);
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
		//dbg("BATTERY ends, cupcake goes back to CHASE\n");
		c_cc->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
		resultingForce = Vector3().Zero;
		repulsionTimer = 0.f;
		//ChangeState("CHASE");
		return LEAVE;
	}

}

int CBTCupcake_explosive::actionRecoilReceived() {
	//retroceder
	TCompTransform* c_trans = get<TCompTransform>();

	VEC3 dir = VEC3();
	dir = c_trans->getFront() * -stunSpeed;


	if (firstImpact) {//JUMP
		firstImpact = false;
		TCompRigidBody* r_bd = get<TCompRigidBody>();
		dir.y = recoilJumpForce;
		r_bd->addForce(dir);
	}
	else if (isGrounded()) {//FALL
		firstImpact = true;
		return LEAVE;
	}
	return STAY;
}

int CBTCupcake_explosive::actionImpactReceived() {
	//retroceder
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
	else if (!isGrounded()) {//FALL

		dir *= dt;

		//UPDATE

		TCompCollider* c_cc = get<TCompCollider>();
		if (c_cc) {
			c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
		}

	}
	else {

		timerStun = limitTimeStun;
		return LEAVE;
	}

	return STAY;
}

//--------------CONDITIONS---------------


bool CBTCupcake_explosive::conditionView() {
 	if (player_dead) {
		return false; //no lo ve si esta muerto
	}
	//return true;//temporal para el milestone 2 
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	if (Vector3::Distance(player_position->getPosition(), c_trans->getPosition()) > forgetAttackingDistance) {
		insightPlayer = false; //TODO: esto es temporal para el milestone 2 
	}

	if (!checkHeight()) { //si el player esta muy alto no lo ve y se olvida de el
		return false;
	}

	if (isView(length_cone) || insightPlayer || enemyRadiousView > Vector3::Distance(player_position->getPosition(), c_trans->getPosition())) {
		insightPlayer = true;
		return true;
	}

	//------------------------------------ Blackboard
	if (inBlackboard) { //si no ve al player estando vivo deja de estar atacando, de modo que libera el slot
		inBlackboard = false;
		slotsAvailable = false;
		CEntity* e_player = (CEntity *)h_player;
		TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
		c_bb->forgetPlayer(CHandle(this).getOwner(), CUPCAKE);
	}
	//------------------------------------

	return false;
}

bool CBTCupcake_explosive::conditionWptClose() {
	if (_curve == nullptr) {
		if (initialExecution) {
			//initialExecution = false;
			//TCompTransform* c_trans = get<TCompTransform>();
			//VEC3 position = c_trans->getPosition();
			//wtp 
			//positions.push_back(h_player.);
			positions.push_back(VEC3(initialPos.x + 5.0f, initialPos.y + 0.0f, initialPos.z + 0.0f));
			positions.push_back(VEC3(initialPos.x - 5.0f, initialPos.y + 0.0f, initialPos.z + 0.0f));
			positions.push_back(VEC3(initialPos.x + 0.0f, initialPos.y + 0.0f, initialPos.z - 5.0f));
			positions.push_back(VEC3(initialPos.x + 0.0f, initialPos.y + 0.0f, initialPos.z + 5.0f));

			nextPoint = positions[wtpIndex];
		}
	}
	else {
		if (initialExecution) {
			//initialExecution = false;
			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 position = c_trans->getPosition();
			_knots = _curve->_knots;
		}
		
		if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}
		ratio += dt * mTravelTime;
		nextPoint = _curve->evaluate(ratio);
		
		//wtpIndex = (wtpIndex + 1) % _knots.size();
		//nextPoint = _knots[wtpIndex];
	}


	TCompTransform* c_trans = get<TCompTransform>();
	if (Vector3::Distance(nextPoint, c_trans->getPosition()) < distanceCheckThreshold) {
		return true;
	}
	return false;
}

bool CBTCupcake_explosive::conditionExplode() {

	//comprobar distancia con el player

	return isView(distanceExplosion) && isGrounded() && !has_exploded;

};

bool CBTCupcake_explosive::conditionDeath() {
	return life <= 0.f;
}


bool CBTCupcake_explosive::conditionGravityReceived() {
	return beingAttracted && battery_time > 0;
	
}

bool CBTCupcake_explosive::conditionTimer() {
	return evaluateState <= 0;
	
}

bool CBTCupcake_explosive::conditionImpactReceived() {
	//recibir daño de algo 
	if (impacted) {
		impacted = false;
		return true;
	}
	return false;
}

bool CBTCupcake_explosive::conditionRecoilReceived() {
	//recibir daño de algo 
	if (recoiled) {
		recoiled = false;
		return true;
	}
	return false;
}

//pasive

void CBTCupcake_explosive::onCollision(const TMsgOnContact& msg) { //no se utiliza 
	if (isPaused()) {
		return;
	}
	CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
	if (source_of_impact) {
		TCompCollider* c_tag = source_of_impact->get<TCompCollider>();
		if (c_tag) {
			PxShape* colShape;
			c_tag->actor->getShapes(&colShape, 1, 0);
			PxFilterData col_filter_data = colShape->getSimulationFilterData();

			if (col_filter_data.word0 & EnginePhysics.Player) {
				if (jump) { //IF YOU ARE HIDE YOU CANNOT SEND DAMAGE
				 // dbg("cupcake collision with player \n");
					recoiled = true;
					//SEND MSG TO PLAYER

					if (damageTimer <= 0) {
						damageTimer = damageDelay;
						Send_DamageMessage<TMsgDamage>(source_of_impact, currentDamage);
					}
				}
				return;
      }
      else if (col_filter_data.word0 & EnginePhysics.Enemy && battery_time > 0) {
          TMsgDamage msg;
          // Who sent this bullet
          msg.h_sender = CHandle(this).getOwner();
          msg.h_bullet = CHandle(this).getOwner();
          msg.intensityDamage = 0.f;
          msg.senderType = PLAYER;
          msg.targetType = ENEMIES;
          msg.impactForce = 2.f;
          source_of_impact->sendMsg(msg);
      }
		}
	}
}

void CBTCupcake_explosive::onDamageToAll(const TMsgDamageToAll& msg) {
	if (isPaused()) {
		return;
	}
	life = life - msg.intensityDamage;
  //FluidDecalGenerator.generateFluid(msg.impactForce, my_trans->getPosition());
	//dbg("se recibe el fuego de la pila life = %f\n", life);
	if (life < 0) {
		life = 0;
	}
}

void CBTCupcake_explosive::onGenericDamageInfoMsg(const TMsgDamage& msg) { //TODO: ARREGLAR
	if (isPaused()) {
		return;
	}
	if (jump) {
		TCompTransform* my_trans = get<TCompTransform>();
		if (msg.senderType == ENVIRONMENT) {
			h_sender = msg.h_sender;

			//TCompTransform* my_trans = get<TCompTransform>();
			VEC3 direction_to_damage = my_trans->getPosition() - msg.position;
			direction_to_damage.y = 1.0f;
			direction_to_damage.Normalize();
			TCompRigidBody* c_rbody = get<TCompRigidBody>();
			if (c_rbody)
				c_rbody->addForce(direction_to_damage * msg.impactForce);

		}
		else {
			h_sender = msg.h_sender;
			h_bullet = msg.h_bullet;

			CEntity* e_bullet = (CEntity *)h_bullet;
			TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
			bullet_position = bullet_trans->getPosition();
			bullet_front = bullet_trans->getFront();
			//impacted = true;

			//
			VEC3 direction_to_damage;
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

		}
		life -= msg.intensityDamage;
    FluidDecalGenerator.generateFluid(msg.impactForce, my_trans->getPosition());
		if (life < 0) {
			life = 0;
		}	
	}
}


void CBTCupcake_explosive::onGravity(const TMsgGravity& msg) {
	if (isPaused()) {
		return;
	}
	//IF YOU ARE HIDE YOU CANNOT RECEIVE DAMAGE
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

	}if (beingAttracted && h_bullet == msg.h_bullet) {

		//obtener posicion
		beingAttracted = true;
		CEntity* e_bullet = (CEntity *)h_bullet;
		TCompTransform* bullet_trans = e_bullet->get<TCompTransform>();
		battery_position = bullet_trans->getPosition();

	}
}

template <typename T>
void CBTCupcake_explosive::Send_DamageMessage(CEntity* entity, float dmg) {
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

void CBTCupcake_explosive::onCreated(const TMsgEntityCreated& msg) {
	TCompTransform* c_trans = get<TCompTransform>();

	initialPos = c_trans->getPosition();
  voice = EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Voice3D");
  voice.set3DAttributes(*c_trans);
}

void CBTCupcake_explosive::onBlackboardMsg(const TMsgBlackboard& msg) {

	player_dead = msg.player_dead;
	if (!player_dead) {
		slotsAvailable = msg.is_posible;
	}
	else {
		slotsAvailable = false;
	}
}

void CBTCupcake_explosive::registerMsgs() {
	//DECL_MSG(CBTCupcake, TMsgDamageToEnemy, onDamageInfoMsg);
	DECL_MSG(CBTCupcake_explosive, TMsgBlackboard, onBlackboardMsg);
	DECL_MSG(CBTCupcake_explosive, TMsgDamage, onGenericDamageInfoMsg);
	DECL_MSG(CBTCupcake_explosive, TMsgOnContact, onCollision);
	DECL_MSG(CBTCupcake_explosive, TMsgGravity, onGravity);
	DECL_MSG(CBTCupcake_explosive, TMsgBTPaused, onMsgBTPaused);
	DECL_MSG(CBTCupcake_explosive, TMsgDamageToAll, onDamageToAll);
	DECL_MSG(CBTCupcake_explosive, TMsgFireAreaEnter, onFireAreaEnter);
	DECL_MSG(CBTCupcake_explosive, TMsgFireAreaExit, onFireAreaExit);
	DECL_MSG(CBTCupcake_explosive, TMsgEntityCreated, onCreated);
	DECL_MSG(CBTCupcake_explosive, TMsgSpawnerCheckin, onCheckin);
	DECL_MSG(CBTCupcake_explosive, TMSgTriggerFalloutDead, onTriggerFalloutDead);
}

//On death send TMsgSpawnerCheckout message to this CHandle
void CBTCupcake_explosive::onCheckin(const TMsgSpawnerCheckin& msg) {
	if (isPaused()) {
		return;
	}
	_mySpawner = msg.spawnerHandle;
}


void CBTCupcake_explosive::onFireAreaEnter(const TMsgFireAreaEnter& msg) {
	//dbg("Cupcake stepped in the fire!\n");
	if (isPaused()) {
		return;
	}

	TCompTransform* c_trans = get<TCompTransform>();
	c_trans->setScale(fireScale); //se vuelve el doble de grande
	currentDamage = fireDamage; //y un 50% mas fuerte
}

void CBTCupcake_explosive::onFireAreaExit(const TMsgFireAreaExit& msg) {
	if (isPaused()) {
		return;
	}
 //descomentar esto si queremos que el cupcake vuelva a su forma original
/*
TCompTransform* c_trans = get<TCompTransform>();
c_trans->setScale(normalScale); //todo vuelve a la normalidad
currentDamage = damage;
*/

}


void CBTCupcake_explosive::onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg) {
	life -= msg.damage;
	isDeadForFallout = msg.falloutDead;
	if (life < 0) {
		life = 0;
	}
}

bool CBTCupcake_explosive::isGrounded() {
	TCompRigidBody* r_body = get<TCompRigidBody>();
	return r_body->is_grounded;
}

void CBTCupcake_explosive::load(const json& j, TEntityParseContext& ctx) {


	damage = j.value("damage", damage);
	fireDamage = j.value("fireDamage", fireDamage);
	impactForceAttack = j.value("impactForceAttack", impactForceAttack);

	life = j.value("life", life);
	spawnRange = j.value("spawnRange", spawnRange);
	jumpingCupcake = j.value("jumpingCupcake", jumpingCupcake);
	impactForceAttack = j.value("impactForceAttack", impactForceAttack);
	limitTimeStun = j.value("limitTimeStun", limitTimeStun);
	changeStateProb = j.value("changeStateProb", changeStateProb);
	normalScale = j.value("normalScale", normalScale);
	fireScale = j.value("fireScale", fireScale);
	evaluateStateTime = j.value("evaluateStateTime", evaluateStateTime);
	evaluateAttack = j.value("evaluateAttack", evaluateAttack);
	attractionForce = j.value("attractionForce", attractionForce);
	repulsionDuration = j.value("repulsionDuration", repulsionDuration);
	jumpForce = j.value("jumpForce", jumpForce);
	recoilJumpForce = j.value("recoilJumpForce", recoilJumpForce);
	distanceCheckThreshold = j.value("distanceCheckThreshold", distanceCheckThreshold);
	forgetAttackingDistance = j.value("forgetAttackingDistance", forgetAttackingDistance);
	stunSpeed = j.value("stunSpeed", stunSpeed);
	speed = j.value("speed", speed);
	hideSpeed = j.value("hideSpeed", hideSpeed);
	twistSpeed = j.value("twistSpeed", twistSpeed);
	batterySpeed = j.value("batterySpeed", batterySpeed);
	evaluateStateTime = j.value("impulseStrenghtSecondSon", impulseStrenghtSecondSon);


	cone_vision = j.value("cone_vision", cone_vision);
	enemyRadiousView = j.value("enemyRadiousView", enemyRadiousView);
	view_point = j.value("view_point", view_point);
	length_cone = j.value("length_cone", length_cone);
	half_cone = j.value("half_cone", half_cone);

	//navmesh values 
	use_navmesh = j.value("use_navmesh", use_navmesh);
	reevaluatePathDelay = j.value("reevaluatePathDelay", reevaluatePathDelay);
	//end navmesh values


	//explosion values
	//explosionActive = j.value("explosionActive", explosionActive);
	distanceExplosion = j.value("distanceExplosion", distanceExplosion);
	explosionDelay = j.value("explosionDelay", explosionDelay);
	explosion_damage = j.value("explosion_damage", explosion_damage);
	explosionRadius = j.value("explosionRadius", explosionRadius);



	this->create("Cupcake_explosive_bt");

}

void CBTCupcake_explosive::movement(VEC3 target, bool seek) {

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 dir = VEC3();

	//MOVE
	if (isGrounded() && (jump || seek)) {//IF GROUNDED AND JUMPING CUPCAKE CANNOT MOVE, JUST TURN
		dir = c_trans->getFront();
	}
	else {
		dir = c_trans->getFront() * speed;
	}

	if (!jump && isGrounded() && !seek) {
		dir = c_trans->getFront() * hideSpeed;
		TCompRender* crender = get<TCompRender>();
		crender->showMeshesWithState(1);
	}
	else {
		TCompRender* crender = get<TCompRender>();
		crender->showMeshesWithState(0);
	}

	//GRAVITY
	dir *= dt;

	//TURN FRONT
	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);

	if (isGrounded()) {
		/*
			if (evaluateState <= 0) {
			evaluateState = 2.f;
			if (jumpingCupcake) { //esto estara dentro de un random
				jumpingCupcake = false;
			}
			else {
				jumpingCupcake = true;
			}
		}
		*/

		//TURN:
		float angle = rad2deg(c_trans->getDeltaYawToAimTo(target));
		if (abs(angle) <= cone_vision) {
			view_point = true;
		}
		else {
			c_trans->rotateTowards(target, twistSpeed, dt);
		}
	}

	// JUMP
	if (isGrounded() && view_point && (jump || seek)) {
		TCompRigidBody* c_rb = get<TCompRigidBody>();
		c_rb->jump(VEC3(0, jumpForce, 0));
		view_point = false;
	}

	//UPDATE DIR
	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc) {
		c_cc->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, dt, PxControllerFilters());
	}
}

bool CBTCupcake_explosive::isView(float distance) {
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

void CBTCupcake_explosive::generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc)
{
	navmeshPath = EngineNavmesh.findPath(initPos, destPos);
	navmeshPathPoint = 0;
	recalculateNavmesh = recalc;

}

void CBTCupcake_explosive::updateBT() {

	//--------------------------navmesh

		TCompTransform* c_trans = get<TCompTransform>();
	if (nextNavMeshPoint != VEC3().Zero	&& use_navmesh){ //update path point
		if (Vector3::Distance(nextNavMeshPoint, c_trans->getPosition()) < distanceCheckThreshold) {
			navMeshIndex++;
			if (navMeshIndex < navmeshPath.size()) {
				nextNavMeshPoint = navmeshPath[navMeshIndex];
			}
			
		}
	}



	damageTimer -= dt;
	reevaluatePathTimer -= dt;

  voice.set3DAttributes(*c_trans);
}

void CBTCupcake_explosive::setCurve(const CCurve* curve) {

	this->_curve = curve; // TO TEST
	_knots = _curve->_knots;
	this->pathCurve = curve->getName();
}

std::string CBTCupcake_explosive::getNameCurve() {
	return pathCurve;
}

bool CBTCupcake_explosive::checkHeight() {
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
