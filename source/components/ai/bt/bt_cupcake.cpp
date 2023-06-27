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
#include "bt_cupcake.h"
#include "components/ai/others/comp_blackboard.h"
#include "components/ai/others/self_destroy.h"
#include "components/vfx/comp_death_billboard.h"
#include "random"

using namespace physx;
std::mt19937 bt_cup(1942);
std::uniform_int_distribution<int> bt_range(0, 100);

DECL_OBJ_MANAGER("bt_cupcake", CBTCupcake);
void CBTCupcake::create(string s)//crear el arbol
{
	name = s; //createRoot y addChild debe hacerse en orden, sino peta -- nunca definir un huerfano directamente 
	// padre - hijo - tipo - condition - action

	if (_initiallyPaused) {
		setPaused(true);
	}

	createRoot("CUPCAKE", PRIORITY, NULL, NULL);

	//pasive states:

	addChild("CUPCAKE", "ON_DIVIDE", ACTION, (btcondition)&CBTCupcake::conditionDivide, (btaction)&CBTCupcake::actionDivide);
	addChild("CUPCAKE", "ON_DEATH", ACTION, (btcondition)&CBTCupcake::conditionDeath, (btaction)&CBTCupcake::actionDeath);
  addChild("CUPCAKE", "DEATH", ACTION, (btcondition)& CBTCupcake::conditionDeathAnimation, (btaction)& CBTCupcake::actionDeathStay);
	addChild("CUPCAKE", "ON_GRAVITY", ACTION, (btcondition)&CBTCupcake::conditionGravityReceived, (btaction)&CBTCupcake::actionGravityReceived);
	//addChild("CUPCAKE", "ON_FEAR", ACTION, (btcondition)&CBTCupcake::conditionFear, (btaction)&CBTCupcake::actionFear);
	addChild("CUPCAKE", "ON_IMPACT", ACTION, (btcondition)&CBTCupcake::conditionImpactReceived, (btaction)&CBTCupcake::actionImpactReceived);
	addChild("CUPCAKE", "ON_RECOIL", ACTION, (btcondition)&CBTCupcake::conditionRecoilReceived, (btaction)&CBTCupcake::actionRecoilReceived);

	//active states
	addChild("CUPCAKE", "INSIGHT", PRIORITY, (btcondition)&CBTCupcake::conditionView, NULL); //es un priority con una condicion random
	addChild("CUPCAKE", "CHANGEWPT", ACTION, (btcondition)&CBTCupcake::conditionWptClose, (btaction)&CBTCupcake::actionChangeWpt);
	addChild("CUPCAKE", "SEEKWPT", ACTION, NULL, (btaction)&CBTCupcake::actionSeekWpt);


	//crear un nodo aqui que sea cambiar de estado de salto a oculto cada x tiempo y que el tiempo sea la condicion de entrada
	addChild("INSIGHT", "CHANGESTATE", ACTION, (btcondition)&CBTCupcake::conditionTimer, (btaction)&CBTCupcake::actionChangeState);
	addChild("INSIGHT", "ORBIT", PRIORITY, (btcondition)&CBTCupcake::conditionRandom, NULL);
	addChild("INSIGHT", "ATTACK", ACTION, NULL, (btaction)&CBTCupcake::actionAttack); //falta el action que no es mas que el chase

	addChild("ORBIT", "CHANGEPOINT", ACTION, (btcondition)&CBTCupcake::conditionOrbitPointClose, (btaction)&CBTCupcake::actionGeneratePoint); //genera los puntos y sigue el mas cercano
	addChild("ORBIT", "FOLLOWPOINT", ACTION, NULL, (btaction)&CBTCupcake::actionFollowPoint);  //seguir un punto


	if (!h_player.isValid()) {
		h_player = GameController.getPlayerHandle();
	}
    voice = EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Voice3D");

}

//--------------ACTIONS---------------
int CBTCupcake::actionChangeWpt() {
	//accion de cambiar de wpt
  //dbg("cambiando de waypoint\n");
	if (_curve == nullptr) {
		wtpIndex = (wtpIndex + 1) % positions.size();
		nextPoint = positions[wtpIndex]; 
	}
	else {
		if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}
		ratio += dt * mTravelTime;
		nextPoint = _curve->evaluate(ratio);
	}


	/*wtpIndex = (wtpIndex + 1) % positions.size();
	nextPoint = positions[wtpIndex];*/
	attacking = false;
	return LEAVE;
}

int CBTCupcake::actionChangeState() {
	evaluateState = evaluateStateTime;

	//random sobre 100
	if (bt_range(bt_cup) < changeStateProb) {
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

int CBTCupcake::actionSeekWpt() {
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

int CBTCupcake::actionAttack() {
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

int CBTCupcake::actionGeneratePoint() {
	//dbg("generando siguiente punto \n");
	generatePoints(); //Update points
	orbitIndex = (orbitIndex + 1) % orbitPositions.size();
	nextOrbitPoint = orbitPositions[orbitIndex];
	orbited_waypoints++;
	float random = rand() % 100;
	if (random <= stopOrbitProbability) {
		orbitting = false;
	}

	return LEAVE;
}

int CBTCupcake::actionFollowPoint() {
	//Accion de seguir un punto generado alredeodr del player
	evaluateState -= dt;
	evaluateAttack -= dt;


	if (use_navmesh) {
		if (reevaluatePathTimer <= 0) {
			reevaluatePathTimer = reevaluatePathDelay;

			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 position = c_trans->getPosition();
			//wtp 
			generateNavmesh(position, nextOrbitPoint, false);

			if (navmeshPath.size() > 0) {
				navMeshIndex = 0;
				nextNavMeshPoint = navmeshPath[navMeshIndex];
			}

		}

		movement(nextNavMeshPoint, true);
	}
	else {
		movement(nextOrbitPoint, true);
	}

	return LEAVE;
}

int CBTCupcake::actionDeath() {
    voice.stop();
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
  if (!isDeadForFallout && !isDeadForTrigger) {
	  GameController.spawnPuddle(c_trans->getPosition(), c_trans->getRotation(), 1.0f);
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
  }
  else {
      AudioEvent death = EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Death3D");
      death.set3DAttributes(*c_trans);
      voice.stop();
	  /*CHandle(this).getOwner().destroy();
	  CHandle(this).destroy();*/
  }
	return LEAVE;
}

int CBTCupcake::actionDeathStay() {
    voice.stop();

  return STAY;
}

int CBTCupcake::actionDivide() {
	TCompTransform* c_trans = get<TCompTransform>();

	VEC3 posicion = VEC3(c_trans->getPosition().x, c_trans->getPosition().y, c_trans->getPosition().z);
	c_trans->setPosition(posicion);
	num_of_divisions--;
	//GENERATE SONS
	VEC3 pos = c_trans->getPosition();
	// float randX;
	 //float randZ;
	int i = 0;
	//int invertir = 1;
	float sonScale = 0.6;
	//isSon = false;
	float posY = 1.0f;
	VEC3 impulse = c_trans->getFront()*impulseStrenghtSecondSon;
    AudioEvent death = EngineAudio.playEvent("event:/Enemies/Cupcake/Cupcake_Death3D");
    death.set3DAttributes(*c_trans);
    voice.stop();

	while (i < num_of_sons) {
		TEntityParseContext ctx;
		c_trans->setPosition(VEC3(pos.x, pos.y + posY, pos.z));
		posY = posY * 2;
		c_trans->setScale(sonScale);
		ctx.root_transform = *c_trans;
		parseScene("data/prefabs/enemies/bt_cupcake.json", ctx);
		i++;
		//enviar mensaje con los hijos y la vida

		TMsgToCupcakeSon msg;
		msg.num_of_sons = 0;
		msg.life = sonLife;
		msg.impulse = impulse;
		msg.insightPlayer = insightPlayer;
		msg.attacking = attacking;
    msg.damage = damage / 3.0f;
		ctx.entities_loaded[0].sendMsg(msg);

	}


	return LEAVE;
}

int CBTCupcake::actionGravityReceived() { //TODO : COMPROBAR QUE ESTO FUNCIONE CUANDO LA BATERIA ESTE ARREGLADA :D
	if (life <= 0) {
    state = "OTHER";
    voice.stop();
    return LEAVE;
	}
  state = "ON_GRAVITY";

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
		if (life <= 0) {
            voice.stop();
            return LEAVE;
		}
		return STAY;
	}
	else {
		beingAttracted = false;
		//dbg("BATTERY ends, cupcake goes back to CHASE\n");
		c_cc->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
		resultingForce = Vector3().Zero;
		repulsionTimer = 0.f;
		
    state = "OTHER";
		return LEAVE;
	}

}

int CBTCupcake::actionRecoilReceived() {
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

	if (life <= 0) {
        voice.stop();
        return LEAVE;
	}

	return STAY;
}

int CBTCupcake::actionImpactReceived() {
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

	if (life <= 0) {
        voice.stop();
        return LEAVE;
	}

	return STAY;
}

//--------------CONDITIONS---------------


bool CBTCupcake::conditionView() {
	if (player_dead) {
		return false; //no lo ve si esta muerto
	}
	//return true;//temporal para el milestone 2 
	TCompTransform* c_trans = get<TCompTransform>();
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	//if (Vector3::Distance(player_position->getPosition(), c_trans->getPosition()) > forgetAttackingDistance) {
		//insightPlayer = false; //TODO: esto es temporal para el milestone 2 
	//}

	bool res = false;


	if (isView(length_cone) || insightPlayer || orbitting || enemyRadiousView > Vector3::Distance(player_position->getPosition(), c_trans->getPosition())) {
		insightPlayer = true;
		res = true;
	}

	if (!checkHeight()) { //si el player esta muy alto no lo ve y se olvida de el
		res = false;
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

	return res;
}

bool CBTCupcake::conditionWptClose() {
	if (_curve == nullptr) {
		if (initialExecution) {
			//initialExecution = false;
			//TCompTransform* c_trans = get<TCompTransform>();
			//VEC3 position = c_trans->getPosition();
			//wtp 
			//positions.push_back(h_player.);

			TCompTransform* c_trans = get<TCompTransform>();
			VEC3 position = c_trans->getPosition();
			positions.push_back(position);
			/*positions.push_back(VEC3(initialPos.x + 5.0f, initialPos.y + 0.0f, initialPos.z + 0.0f));
			positions.push_back(VEC3(initialPos.x - 5.0f, initialPos.y + 0.0f, initialPos.z + 0.0f));
			positions.push_back(VEC3(initialPos.x + 0.0f, initialPos.y + 0.0f, initialPos.z - 5.0f));
			positions.push_back(VEC3(initialPos.x + 0.0f, initialPos.y + 0.0f, initialPos.z + 5.0f));
			*/
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
		//wtpIndex = (wtpIndex + 1) % _knots.size();
		//nextPoint = _knots[wtpIndex];

		//MODIFICACION DE CODIGO 
		if (ratio >= 1.0f || ratio < 0.0f) {
			mTravelTime = -mTravelTime;
		}
			ratio += dt * mTravelTime;
			nextPoint = _curve->evaluate(ratio);
		//HASTA AQUI
	}




	TCompTransform* c_trans = get<TCompTransform>();
	if (Vector3::Distance(nextPoint, c_trans->getPosition()) < distanceCheckThreshold) {
		return true;
	}
	return false;
}

bool CBTCupcake::conditionOrbitPointClose() {

	TCompTransform* c_trans = get<TCompTransform>();
	
	if (nextOrbitPoint == VEC3().Zero) { //si no hay punto que lo genere
		return true;
	}

	if (Vector3::Distance(nextOrbitPoint, c_trans->getPosition()) < distanceCheckThreshold  && life > 0 ) {//|| attacking
		noOrbitPoint = false;
		generatePoints(); //generar puntos
		orbitIndex = (orbitIndex + 1) % orbitPositions.size();
		nextOrbitPoint = orbitPositions[orbitIndex];
		orbitting = true;
		attacking = false;
		return true;
	}
	return false;
}

bool CBTCupcake::conditionRandom() {
	//RANDOM DE ORBITAR AL PLAYER TRUE SI ORBITA FALSE SI ATACA
	//-----------------------------------------------
	//return false;//descativar despues del milestone 2
	if (inBlackboard) { //si ya esta en la blackboard sigue atacando
		return false; //attack
	}
	else {
		CEntity* e_player = (CEntity *)h_player;
		TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
		slotsAvailable = c_bb->checkPermission(CHandle(this).getOwner(), CUPCAKE);

		if (!slotsAvailable) { //si no hay espacio no ataca
			return true; //not attack
		}
		else { //si hay espacio ataca
			inBlackboard = true;
			return false; //attack
		}
	}

	//-----------------------------------------------
	/* //no es necesario con la blackboard
	if (orbited_waypoints >= max_orbited_points) {
		orbited_waypoints = 0;
		return false; //attack
	}*/

	/* //no es necesario con la blackboard 
	if (evaluateAttack <= 0) {
		//dbg("evaluamos si atacar o no \n");
		evaluateAttack = evaluateAttackValue;
		if (bt_range(bt_cup) >= orbitProbability) {//&& !attacking
			return false;//atack
		}
		else {//ceder el slot a otro enemigp
			//------------------------------------ Blackboard
			if (inBlackboard) { //si no ve al player estando vivo deja de estar atacando, de modo que libera el slot
				inBlackboard = false;
				slotsAvailable = false;
				CEntity* e_player = (CEntity *)h_player;
				TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
				c_bb->forgetPlayer(CHandle(this).getOwner(), CUPCAKE);
			}
			//------------------------------------
			return true; //orbit
		}
	}
	if (attacking) {
		return false;
	}
	else {
		return true;
	}*/

}

bool CBTCupcake::conditionDeath() {
	return life <= 0.f && !death_animation_started;
}

bool CBTCupcake::conditionDeathAnimation() {
  return life <= 0.f && death_animation_started;
}

bool CBTCupcake::conditionDivide() {
	return life <= 0.f && num_of_divisions > 0;
}

bool CBTCupcake::conditionGravityReceived() {
	return beingAttracted && battery_time > 0;
	
}

bool CBTCupcake::conditionTimer() { 
	return evaluateState <= 0;
	
}

bool CBTCupcake::conditionImpactReceived() {
	//recibir daño de algo 
	if (impacted) {
		impacted = false;
		return true;
	}
	return false;
}

bool CBTCupcake::conditionRecoilReceived() {
	//recibir daño de algo 
	if (recoiled) {
		recoiled = false;
		return true;
	}
	return false;
}

std::string CBTCupcake::getState() {
  return state;
}

//pasive

void CBTCupcake::onCollision(const TMsgOnContact& msg) { //no se utiliza 
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

			if (col_filter_data.word0 & EnginePhysics.Player || col_filter_data.word0 & EnginePhysics.Product) {
				if (jump) { //IF YOU ARE HIDE YOU CANNOT SEND DAMAGE
				 // dbg("cupcake collision with player \n");
					recoiled = true;
					//SEND MSG TO PLAYER

					if (damageTimer <= 0 && getState() != "ON_GRAVITY" ) {
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

void CBTCupcake::onDamageToAll(const TMsgDamageToAll& msg) {
	if (isPaused() || invulnerable) {
		return;
	}
	life = life - msg.intensityDamage;
  //FluidDecalGenerator.generateFluid(msg.impactForce, my_trans->getPosition());
	//dbg("se recibe el fuego de la pila life = %f\n", life);
	if (life < 0.0f) {
		life = 0.0f;
        voice.stop();
	}
}

void CBTCupcake::onGenericDamageInfoMsg(const TMsgDamage& msg) { //TODO: ARREGLAR
	if (isPaused() || invulnerable) {
		return;
	}
	if (jump) {
		TCompTransform* my_trans = get<TCompTransform>();
		life -= msg.intensityDamage;
		FluidDecalGenerator.generateFluid(msg.impactForce, my_trans->getPosition());
		if (life < 0.0f) {
			life = 0.0f;
            voice.stop();
        }
		if (msg.senderType == ENVIRONMENT) {
			h_sender = msg.h_sender;

			//TCompTransform* my_trans = get<TCompTransform>();
			VEC3 direction_to_damage = my_trans->getPosition() - msg.position;
			direction_to_damage.y = 1.0f;
			direction_to_damage.Normalize();
			TCompRigidBody* c_rbody = get<TCompRigidBody>();
			if (c_rbody && life > 0) {
				c_rbody->addForce(direction_to_damage * msg.impactForce);
			}
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
			direction_to_damage.y = 0.0f;
			direction_to_damage.Normalize();
			TCompRigidBody* c_rbody = get<TCompRigidBody>();
			if (c_rbody  && life > 0) {
				c_rbody->addForce(direction_to_damage * msg.impactForce * 0.5);
			}

		}
	}
}

void CBTCupcake::onTMsgToCupcakeSon(const TMsgToCupcakeSon& msg) {
	if (isPaused()) {
		return;
	}
	//dbg("recibe el mensaje al nacer\n");
	num_of_sons = msg.num_of_sons;
	life = msg.life;
	insightPlayer = msg.insightPlayer;
	attacking = msg.attacking;
  damage = msg.damage;

	TCompRigidBody* c_rb = get<TCompRigidBody>();
	c_rb->addForce(msg.impulse);

	TCompCollider* c_cc = get<TCompCollider>();
	if (c_cc) {
		c_cc->controller->resize(sonHeight); //original = 0.8
		c_cc->controller->setRadius(sonRadius); //original = 0.5
	}
}

void CBTCupcake::onGravity(const TMsgGravity& msg) {
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
void CBTCupcake::Send_DamageMessage(CEntity* entity, float dmg) {
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

void CBTCupcake::onCreated(const TMsgEntityCreated& msg) {
	
}

void CBTCupcake::onBlackboardMsg(const TMsgBlackboard& msg) {

	player_dead = msg.player_dead;
	if (!player_dead) {
		inBlackboard = msg.is_posible; 
	}
	else {
		inBlackboard = false;
	}
}

void CBTCupcake::registerMsgs() { 
	//DECL_MSG(CBTCupcake, TMsgDamageToEnemy, onDamageInfoMsg);
	DECL_MSG(CBTCupcake, TMsgBlackboard, onBlackboardMsg);
	DECL_MSG(CBTCupcake, TMsgDamage, onGenericDamageInfoMsg);
	DECL_MSG(CBTCupcake, TMsgOnContact, onCollision);
	DECL_MSG(CBTCupcake, TMsgGravity, onGravity);
	DECL_MSG(CBTCupcake, TMsgBTPaused, onMsgBTPaused);
	DECL_MSG(CBTCupcake, TMsgToCupcakeSon, onTMsgToCupcakeSon);
	DECL_MSG(CBTCupcake, TMsgDamageToAll, onDamageToAll);
	DECL_MSG(CBTCupcake, TMsgFireAreaEnter, onFireAreaEnter);
	DECL_MSG(CBTCupcake, TMsgFireAreaExit, onFireAreaExit);
	DECL_MSG(CBTCupcake, TMsgEntityCreated, onCreated);
	DECL_MSG(CBTCupcake, TMsgSpawnerCheckin, onCheckin);
	DECL_MSG(CBTCupcake, TMSgTriggerFalloutDead, onTriggerFalloutDead);
	DECL_MSG(CBTCupcake, TMsgDeleteTrigger, onDeleteTrigger);

}

//On death send TMsgSpawnerCheckout message to this CHandle
void CBTCupcake::onCheckin(const TMsgSpawnerCheckin& msg) {
	if (isPaused()) {
		return;
	}
	_mySpawner = msg.spawnerHandle;
}


void CBTCupcake::onFireAreaEnter(const TMsgFireAreaEnter& msg) {
	//dbg("Cupcake stepped in the fire!\n");
	if (isPaused()) {
		return;
	}

	TCompTransform* c_trans = get<TCompTransform>();
	c_trans->setScale(fireScale); //se vuelve el doble de grande
	//currentDamage = fireDamage; //y un 50% mas fuerte
}

void CBTCupcake::onFireAreaExit(const TMsgFireAreaExit& msg) {
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


void CBTCupcake::onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg) {
	life -= msg.damage;
	isDeadForFallout = msg.falloutDead;
	if (life < 0.f) {
		life = 0.0f;
        voice.stop();
    }
}



bool CBTCupcake::isGrounded() {
	TCompRigidBody* r_body = get<TCompRigidBody>();
	return r_body->is_grounded;
}

void CBTCupcake::load(const json& j, TEntityParseContext& ctx) {


	damage = j.value("damage", damage);
	fireDamage = j.value("fireDamage", fireDamage);
	impactForceAttack = j.value("impactForceAttack", impactForceAttack);
	num_of_divisions = j.value("num_of_divisions", num_of_divisions);
	life = j.value("life", life);
	sonLife = j.value("sonLife", sonLife);
	num_of_sons = j.value("num_of_sons", num_of_sons);
	spawnRange = j.value("spawnRange", spawnRange);
	orbitRadious = j.value("orbitRadious", orbitRadious);
	minDistanceOrbit = j.value("minDistanceOrbit", minDistanceOrbit);
	number_of_orbit_points = j.value("number_of_orbit_points", number_of_orbit_points);
	jumpingCupcake = j.value("jumpingCupcake", jumpingCupcake);
	impactForceAttack = j.value("impactForceAttack", impactForceAttack);
	limitTimeStun = j.value("limitTimeStun", limitTimeStun);
	changeStateProb = j.value("changeStateProb", changeStateProb);
	normalScale = j.value("normalScale", normalScale);
	fireScale = j.value("fireScale", fireScale);
	evaluateStateTime = j.value("evaluateStateTime", evaluateStateTime);
	evaluateAttack = j.value("evaluateAttack", evaluateAttack);
	sonRadius = j.value("sonRadius", sonRadius);
	sonHeight = j.value("sonHeight", sonHeight);
	max_orbited_points = j.value("max_orbited_points", max_orbited_points);
	attractionForce = j.value("attractionForce", attractionForce);
	repulsionDuration = j.value("repulsionDuration", repulsionDuration);
	orbitProbability = j.value("orbitProbability", orbitProbability);
	stopOrbitProbability = j.value("stopOrbitProbability", stopOrbitProbability);
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


	this->create("Cupcake_bt");

}

void CBTCupcake::movement(VEC3 target, bool seek) {

	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 dir = VEC3();
    voice.set3DAttributes(*c_trans);

	//MOVE
	if (isGrounded() && (jump || seek)) {//IF GROUNDED AND JUMPING CUPCAKE CANNOT MOVE, JUST TURN
		dir = c_trans->getFront();
	}
	else {
		dir = c_trans->getFront() * speed;
	}

	/*if (!jump && isGrounded() && !seek) {    //descomentar esto para que se oculte al orbitar
		dir = c_trans->getFront() * hideSpeed;
		TCompRender* crender = get<TCompRender>();
		crender->showMeshesWithState(1);
	}
	else {
		TCompRender* crender = get<TCompRender>();
		crender->showMeshesWithState(0);
	}*/

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

bool CBTCupcake::isView(float distance) {
	CEntity* e_player = (CEntity *)h_player;
	TCompTransform* player_position = e_player->get<TCompTransform>();
	//float playerHeight = player_position->getPosition().y;
	TCompTransform* c_trans = get<TCompTransform>();
	//float enemyHeight = c_trans->getPosition().y;
	float distancia = VEC3::Distance(c_trans->getPosition(), player_position->getPosition());
	//CONE
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(player_position->getPosition()));
	if (abs(angle) <= half_cone) {
		//DISTANCE
		if (distancia <= distance ) {
			return true;
		}
	}
	return false;
}

bool CBTCupcake::checkHeight() {
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

void CBTCupcake::generatePoints() {
	CEntity* e_player = (CEntity *)h_player;

	TCompTransform* player_position = e_player->get<TCompTransform>();
	VEC3 playerCurrentPos = player_position->getPosition();

	orbitPositions.clear();

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

void CBTCupcake::generateNavmesh(VEC3 initPos, VEC3 destPos, bool recalc)
{
	navmeshPath = EngineNavmesh.findPath(initPos, destPos);
	navmeshPathPoint = 0;
	recalculateNavmesh = recalc;

}


void CBTCupcake::updateBT() {

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
	//----------------------- navmesh


	//---------- hijos invulnerables durante dos segundos y no hacen daño al nacer
	if (num_of_sons == 0) { //si es un hijo
		if (recienNacidoTimer > 0) {
			currentDamage = 0;
			invulnerable = true;
			recienNacidoTimer -= dt;
		}
		else {
			currentDamage = damage;
			invulnerable = false;
		}
	}
	//------------------------

	//check if is in the blackboard
	if (inBlackboard) {
		if (resetSlotTimer >= resetSlotDuration) { //si ha superado el limite de tiempo, libera el slot
			resetSlotTimer = 0.0f;
			inBlackboard = false;
			slotsAvailable = false;
			CEntity* e_player = (CEntity *)h_player;
			TCompBlackboard* c_bb = e_player->get<TCompBlackboard>();
			c_bb->forgetPlayer(CHandle(this).getOwner(), CUPCAKE);
		}
		else {
			resetSlotTimer += dt;
		}
	}


	damageTimer -= dt;
	reevaluatePathTimer -= dt;

  voice.set3DAttributes(*c_trans);
  if (life <= 0) {
      voice.stop();
  }
}


void CBTCupcake::setCurve(const CCurve* curve) {

	this->_curve = curve; // TO TEST
	_knots = _curve->_knots;
	this->pathCurve = curve->getName();
}


std::string CBTCupcake::getNameCurve() {
	return pathCurve;
}



void CBTCupcake::setLengthCone(float length_cone) {
	this->length_cone = length_cone;
}

void CBTCupcake::setHalfCone(float half_cone) {
	this->half_cone = half_cone;
}

void CBTCupcake::setLife(float life_) {
	this->life = life_;
}


void CBTCupcake::renderDebug() {
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

	
	drawCircle(pos, enemyRadiousView, c_render->color);
	drawLine(pos, pos + half_cone_1 * length_cone, c_render->color);
	drawLine(pos, pos + half_cone_2 * length_cone, c_render->color);
	drawLine(pos + half_cone_1 * length_cone, pos + half_cone_2 * length_cone, c_render->color);
	
	
}


void CBTCupcake::onDeleteTrigger(const TMsgDeleteTrigger& msg) {
	isDeadForTrigger = true;
	life = 0.0f;
  voice.stop();
  num_of_divisions = 0;
  TCompSelfDestroy* c_sd = get<TCompSelfDestroy>();
  c_sd->setDelay(0.25f);
  c_sd->enable();
}

float CBTCupcake::getLife() {
	return life;
}
