#include "mcv_platform.h"
#include "comp_character_controller.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/physics/comp_rigid_body.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_tags.h"
#include "components/common/comp_camera.h"
#include "components/common/comp_render.h"
#include "components/common/comp_name.h"
#include "components/powers/comp_fire.h"
#include "components/powers/comp_coffee.h"
#include "components/powers/comp_teleport.h"
#include "components/powers/comp_madness.h"
#include "components/powers/comp_battery.h"
#include "engine.h"
#include "modules/module_physics.h"
#include "input/module_input.h"
#include "components/ai/others/comp_blackboard.h"
#include "windows/app.h"
#include "components/animation/comp_player_animation.h"
#include "modules/module_camera_mixer.h"
#include "components/controllers/comp_trans_curve_controller.h"
#include "ui/widgets/ui_bar.h"
#include "ui/widgets/ui_image.h"
#include "ui/module_ui.h"
#include "ui/ui_widget.h"
#include "components/objects/comp_enemy_spawner.h"
#include "components/objects/comp_enemy_spawner_special_trap.h"
#include "skeleton/comp_skel_lookat_direction.h"
#include "components/common/comp_dynamic_instance.h"
#include "ui/widgets/ui_button.h"

using namespace physx;

DECL_OBJ_MANAGER("character_controller", TCompCharacterController);

void TCompCharacterController::Init() {
  AddState("GROUNDED", (statehandler)&TCompCharacterController::grounded);
  AddState("DASHING", (statehandler)&TCompCharacterController::dashing);
  AddState("ON_AIR", (statehandler)&TCompCharacterController::onAir);
  AddState("DAMAGED", (statehandler)&TCompCharacterController::damaged);
  AddState("DEAD", (statehandler)&TCompCharacterController::dead);
  AddState("MOUNTED", (statehandler)&TCompCharacterController::mounted);
  AddState("WIN", (statehandler)&TCompCharacterController::win);
  AddState("ATTACKING", (statehandler)&TCompCharacterController::attack);
  AddState("CHARGED_ATTACK", (statehandler)&TCompCharacterController::chargedAttack);
  //AddState("USINGCHILLI", (statehandler)&TCompCharacterController::attackChilli);
  AddState("NOCLIP", (statehandler)&TCompCharacterController::noclip);
  AddState("IDLE_CINEMATIC", (statehandler)& TCompCharacterController::idleCinematic);
  

  AddState("ESPECIAL_CINEMATIC", (statehandler)& TCompCharacterController::specialCinematic);



    //ADD MORE STATES FOR BEING HIT, ETC, ETC

    footSteps = EngineAudio.playEvent("event:/Character/Footsteps/Footsteps");
    footStepsSlow = EngineAudio.playEvent("event:/Character/Footsteps/Footsteps_Slow");
    footSteps.setPaused(true);
    footStepsSlow.setPaused(true);
    damagedAudio = EngineAudio.playEvent("event:/Character/Voice/Player_Pain");
    damagedAudio.stop();


	power_selected = GameController.getPowerSelected();

    ChangeState("GROUNDED");
}

void TCompCharacterController::update(float dt) {
	dt = Time.delta_unscaled;
	if (invulnerabilityTimer > 0) {
		invulnerabilityTimer -= dt;
	}
    if (inCombatTimer > 0) {
        inCombatTimer -= dt;
    }
		
		if (extintorActive == true && extintorMeshTimer > 0) {
			extintorMeshTimer -= dt;
		}
		else if (extintorActive == true) {
		//TODO: PONER LA ULTIMA MESH QUE TUVIERA SELECCIONADA
				power_selected = last_power_selected;
				if (power_selected == PowerType::TELEPORT) {
					changeWeaponMesh(WeaponMesh::SCANNER);
				}
				else if (power_selected == PowerType::BATTERY) {
					changeWeaponMesh(WeaponMesh::BATTERTY);
				}
				else if (power_selected == PowerType::MELEE) {
					changeWeaponMesh(WeaponMesh::MOP);
				}
				extintorActive = false;

		}

	if (!_pausedAI) {
		PROFILE_FUNCTION("IAIController");
		assert(!state.empty());
		assert(statemap.find(state) != statemap.end());
		// this is a trusted jump as we've tested for coherence in ChangeState
		(this->*statemap[state])(Time.delta);
	}
}

void TCompCharacterController::debugInMenu() {
    ImGui::LabelText("State", "%s", state.c_str());
    ImGui::DragFloat("Time between dashes", &time_between_dashes, 0.1f, 0.0f, 5.0f);
    ImGui::DragFloat("Jump Force", &jump_force, 0.5f, 0.f, 130.f);
    ImGui::DragFloat("Double Jump Force", &double_jump_force, 0.5f, 0.f, 130.f);
    ImGui::DragFloat("Falling factor on hover", &falling_factor_hover, 0.1f, 0.f, 1.f);
    ImGui::DragFloat("Moving factor on hover", &moving_factor_hover, 0.1f, 0.f, 1.f);
    ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 10.f);
    ImGui::DragFloat("Dash Speed", &dash_speed, 0.1f, 0.f, 10.f);
    ImGui::DragFloat("Rotation speed", &rotation_speed, 0.1f, 0.f, 10.f);
    ImGui::DragFloat("Life", &life, 0.10f, 0.f, maxLife);
    ImGui::DragFloat("Distance to aim", &distance_to_aim, 0.10f, 0.f, 100.f);
    ImGui::Checkbox("Attacking", &attacking);
   /* ImGui::Checkbox("UnLockable Battery", &unLockableBattery);
	  ImGui::Checkbox("UnLockable Teleport", &unLockableTeleport);
	  ImGui::Checkbox("UnLockable Chilli", &unLockableChilli);
	  ImGui::Checkbox("UnLockable Coffe", &unLockableCoffe);*/
}

void TCompCharacterController::renderDebug() {
  TCompTransform* c_trans = get<TCompTransform>();
  TCompCollider* comp_collider = get<TCompCollider>();
  if (!comp_collider || !comp_collider->controller)
    return;
    if (state == "ATTACKING") {
      Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
      PxF32 attackHeight = comp_collider->controller->getHeight();
      damageOrigin.y = c_trans->getPosition().y + (float)attackHeight;
      drawWiredSphere(damageOrigin, meleeRadius, VEC4(1, 0, 0, 1));
    }
    else if (EngineInput["noclip_"].justPressed() && getState() == "NOCLIP") {
        speed /= 2;
        TCompCollider* comp_collider = get<TCompCollider>();
        if (!comp_collider || !comp_collider->controller)
            return;
        comp_collider->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
        ChangeState("ON_AIR");
        return;
    }
    auto& app = CApplication::get();
    if (EngineInput["exitGame_"].justPressed()) {
        DestroyWindow(app.getHandle());
    }
}

void TCompCharacterController::load(const json& j, TEntityParseContext& ctx) {
    this->Init();

    life = j.value("life", life);
    enabled = j.value("enabled", enabled);
    speed = j.value("speed", speed);
    rotation_speed = j.value("rotation_sensibility", rotation_speed);
    distance_to_aim = j.value("distance_to_aim", distance_to_aim);
	
}

void TCompCharacterController::registerMsgs() {
    DECL_MSG(TCompCharacterController, TMsgOnContact, onCollision);
    DECL_MSG(TCompCharacterController, TMsgDamage, onGenericDamage);
    DECL_MSG(TCompCharacterController, TMsgTrapWind, onTrapWind);
    // DECL_MSG(TCompCharacterController, TMsgDamageToPlayer, onDamage);
    DECL_MSG(TCompCharacterController, TMsgEntityTriggerEnter, onEnter);
    DECL_MSG(TCompCharacterController, TMsgPowerUp, onPowerUp);
    DECL_MSG(TCompCharacterController, TMsgDamageToAll, onDamageAll);
    DECL_MSG(TCompCharacterController, TMsgBatteryDeactivates, onBatteryDeactivation);
    DECL_MSG(TCompCharacterController, TCompPlayerAnimator::TMsgPlayerAnimationFinished, onAnimationFinish);
	  DECL_MSG(TCompCharacterController, TMsgOnCinematic, onCinematic);
	  DECL_MSG(TCompCharacterController, TMSgTriggerFalloutDead, onTriggerFalloutDead);
	  DECL_MSG(TCompCharacterController, TMsgOnCinematicSpecial, onCinematicSpecial);
    DECL_MSG(TCompCharacterController, TMsgMeleeHit, onMeleeHit);
}

void TCompCharacterController::onMeleeHit(const TMsgMeleeHit& msg) {
  if (attacking) {
    TCompTransform* c_trans = get<TCompTransform>();
    TMsgDamage dmgmsg;
    // Who sent this bullet
    dmgmsg.h_sender = CHandle(this).getOwner();
    dmgmsg.h_bullet = CHandle(this).getOwner();
    dmgmsg.position = c_trans->getPosition() + VEC3::Up;
    dmgmsg.senderType = PLAYER;
    dmgmsg.intensityDamage = meleeDamage * comboModifier;
    dmgmsg.impactForce = impactForceAttack * comboModifier;
    dmgmsg.damageType = MELEE;
    dmgmsg.targetType = ENEMIES;
    CHandle(msg.h_entity).sendMsg(dmgmsg);
  }
}

void TCompCharacterController::onAnimationFinish(const TCompPlayerAnimator::TMsgPlayerAnimationFinished& msg) {
    switch (msg.animation)
    {
      case TCompPlayerAnimator::MELEE1_FULL:
      case TCompPlayerAnimator::MELEE1_PARTIAL:
      case TCompPlayerAnimator::MELEE2_FULL:
      case TCompPlayerAnimator::MELEE2_PARTIAL:
        attacking = false;
        break;
      case TCompPlayerAnimator::THROW:
          dbg("Animation THROW callback received.\n");
          isThrowingAnimationGoing = false;
          break;
      case TCompPlayerAnimator::CHARGED_MELEE_ATTACK:
          dbg("Animation CHARGED_MELEE_ATTACK callback received.\n");
          isCHARGED_MELEE_ATTACKGoing = false;
          break;
      case TCompPlayerAnimator::DRINK:
          dbg("Animation CHARGED_MELEE_ATTACK callback received.\n");
          isDRINKGoing = false;
          changeWeaponMesh(WeaponMesh::MOP);
          break;
      default:
          break;
    }
}
//STATES

void TCompCharacterController::idleCinematic(float delta) {
    footSteps.setPaused(true);
    footStepsSlow.setPaused(true);
  //DO NOTHING, ONLY LOOP IDLE
	if (!cinematic) {
		ChangeState("GROUNDED");
		UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
		mirilla->getParams()->visible = false;
	}
}


void TCompCharacterController::specialCinematic(float delta) {	
	float twistSpeed = 10;
	TCompTransform* c_trans = get<TCompTransform>();
	VEC3 dir = VEC3();
	dir = c_trans->getFront() * speedCinematicSpecial;
	dir *= Time.delta_unscaled;
	float yaw, pith;
	c_trans->getAngles(&yaw, &pith);
	float angle = rad2deg(c_trans->getDeltaYawToAimTo(targetTower));
	c_trans->rotateTowards(targetTower, twistSpeed, delta);
	//MOVE PLAYER
	TCompCollider* comp_collider = get<TCompCollider>();
	if (!comp_collider || !comp_collider->controller)
		return;
	TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
	playerAnima->playAnimation(TCompPlayerAnimator::RUN, 1.f, true);
	comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());
	float distancia = VEC3::Distance(targetTower, c_trans->getPosition());
	if (distancia < 0.5) {
		ChangeState("GROUNDED");
	}
}

void TCompCharacterController::grounded(float delta) {

	CEntity* entity = EngineEntities.getInventoryHandle();
	TCompInventory* inventory = entity->get<TCompInventory>();


    if (!enabled)
        return;

    //WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
    if (!h_camera.isValid()) {
        h_camera = getEntityByName("PlayerCamera");
        return;
    }

    treatRumble(Time.delta_unscaled);
    bool startDash = false;
    can_double_jump = true;

    VEC3 dir = VEC3().Zero;

    //GROUNDED, PLAYER CAN AIM, SHOOT, DASH, JUMP AND MOVE
    powerSelection();
    aiming = false;

    //MOVEMENT
    getInputForce(dir);
    if (dir != VEC3().Zero && Time.real_scale_factor != 0.0f) {
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        float front_ = EngineInput["front_"].value;
        //With keyboard "s" back_ == 1.0, with joystick back_ == -1.0
        float back_ = EngineInput["back_"].value;
        float left_ = EngineInput["left_"].value;
        float right_ = EngineInput["right_"].value;
        if (fabs(back_) > 0.f && aiming) {
            //Backwards
            if (fabs(front_) > 0.5f || fabs(back_) > 0.5f || fabs(left_) > 0.5f || fabs(right_) > 0.5f) {
            //if (fabs(back_) <= 0.5f && (fabs(left_) <= 0.5f || fabs(right_) <= 0.5f)) {
                //Run backwards
                playerAnima->playAnimation(TCompPlayerAnimator::RUN, -1.0f);
                //Play sound
                if (footSteps.getPaused()) {
                    footStepsSlow.setPaused(true);
                    footSteps.setPaused(false);
                    footSteps.restart();
                }
            }
            else {
                //Walk backwards
                playerAnima->playAnimation(TCompPlayerAnimator::WALK, -1.0f);
                //Play sound
                if (footStepsSlow.getPaused()) {
                    footSteps.setPaused(true);
                    footStepsSlow.setPaused(false);
                    footStepsSlow.restart();
                }
            }
        }
        else {
            //Forwards
            if (fabs(front_) > 0.4f || fabs(back_) > 0.4f || fabs(left_) > 0.4f || fabs(right_) > 0.4f) {
                //Run Forwards
                playerAnima->playAnimation(TCompPlayerAnimator::RUN, 1.0f);
                //Play sound
                if (footSteps.getPaused()) {
                    footStepsSlow.setPaused(true);
                    footSteps.setPaused(false);
                    footSteps.restart();
                }
            }else{
            //if (fabs(front_) <= 0.5f && (fabs(left_) <= 0.5f || fabs(right_) <= 0.5f)) {
                //Walk Forwards
                playerAnima->playAnimation(TCompPlayerAnimator::WALK, 1.0f);
                //Play sound
                if (footStepsSlow.getPaused()) {
                    footSteps.setPaused(true);
                    footStepsSlow.setPaused(false);
                    footStepsSlow.restart();
                }
            }
        }
        
        //SwapMesh(2);
        
    }
    else {
        //SwapMesh(0);
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
		if(playerAnima != nullptr){
            CEntity* weapon2 = getEntityByName("Anti_extintor");
            TCompRender* w_r2 = weapon2->get<TCompRender>();
            w_r2->is_visible;
            if (w_r2->is_visible) {
                playerAnima->playAnimation(TCompPlayerAnimator::IDLE_FIRE, 1.0f);
            }
            else {
                if (inCombatTimer > 0.f) {
                    playerAnima->playAnimation(TCompPlayerAnimator::IDLE_COMBAT, 1.0f);
                }
                else {
                    playerAnima->playAnimation(TCompPlayerAnimator::IDLE_MELEE, 1.0f);
                }
            }
            
			
			//footSteps.stop();
			if (!footSteps.getPaused()) {
				footSteps.setPaused(true);
			}
            if (!footStepsSlow.getPaused()) {
                footStepsSlow.setPaused(true);
            }
		}
    }

    if (time_to_next_dash > 0.0f)
        time_to_next_dash -= Time.delta_unscaled;

    if (EngineInput["aim_"].isPressed()) {//AIM
        aiming = true;

				if (power_selected == PowerType::TELEPORT && !attacking) {
                    changeWeaponMesh(WeaponMesh::SCANNER);
				}
				else if (power_selected == PowerType::BATTERY && !attacking) {
                    changeWeaponMesh(WeaponMesh::BATTERTY);
				} 
				else if (power_selected == PowerType::FIRE && !attacking) {
					changeWeaponMesh(WeaponMesh::EXTINTOR);
				}
				else if (power_selected == PowerType::MELEE) {
					changeWeaponMesh(WeaponMesh::MOP);
				}
    }
    if (EngineInput["shoot_"].justPressed() && aiming) {//SHOOT
        shoot();
    }
    if (EngineInput["dash_"].justPressed() && time_to_next_dash <= 0.0f) {//DASH
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        playerAnima->playAnimation(TCompPlayerAnimator::DASH, 1.5f);
        ChangeState("DASHING");
        dash = dash_limit;
        startDash = true;
        EngineAudio.playEvent("event:/Character/Other/Dash");
    }
    else if (EngineInput["jump_"].justPressed()) {//JUMP
        TCompRigidBody* c_rbody = get<TCompRigidBody>();
        if (!c_rbody)
            return;
        EngineAudio.playEvent("event:/Character/Footsteps/Jump_Start");
        c_rbody->jump(VEC3(0.0f, jump_force, 0.0f));
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        playerAnima->playAnimation(TCompPlayerAnimator::JUMP_START, 1.0f);
        //Commented because the state machine will transition to it on its own
        //ChangeState("ON_AIR");
    }
    else if (!isGrounded()) { //FALLING
        ChangeState("ON_AIR");
    }
    else if (EngineInput["attack_"].isPressed() && meleeTimer <= 0) {//CHARGED ATTACK
        //If the button is pressed for chargedAttack_buttonPressThreshold or more, player is holding the button
        if (chargedAttack_buttonPressTimer >= chargedAttack_buttonPressThreshold) {
            //Player is holding the button
            ChangeState("CHARGED_ATTACK");
            TCompSkeleton* c_skel = get<TCompSkeleton>();
            //c_skel->clearAnimations();
            changeWeaponMesh(WeaponMesh::MOP);
            TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
            playerAnima->playAnimation(TCompPlayerAnimator::CHARGED_MELEE_POSE, 1.0f);
            if (!footSteps.getPaused()) {
                footSteps.setPaused(true);
            }
            if (!footStepsSlow.getPaused()) {
                footStepsSlow.setPaused(true);
            }
        }
        else {
            chargedAttack_buttonPressTimer += Time.delta_unscaled;
        }
    }
    else if (EngineInput["attack_"].justReleased() && meleeTimer <= 0) {//ATTACK
        if (chargedAttack_buttonPressTimer <= chargedAttack_buttonPressThreshold) {
            chargedAttack_buttonPressTimer = 0.f;
            //Player didn't hold the button
            ChangeState("ATTACKING");
        }
    }
    else if (EngineInput["interact_"].justPressed()) {//INTERACT
        interact();
    }
    else if (EngineInput["coffee_time_"].justPressed() && inventory->getCoffe()) { //COFFEE
      //dbg("switch coffe ground\n");
        TCompCoffeeController* c_coffee = get<TCompCoffeeController>();
        if (!c_coffee->getIsEnabled()) {
            TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
            playerAnima->playAnimation(TCompPlayerAnimator::DRINK, 1.0f, true);
            changeWeaponMesh(WeaponMesh::ENERGIZER);
        }
        c_coffee->switchState();
    }
    else if (EngineInput["fire_attack_"].isPressed() && inventory->getChilli()) { //FIRE
				if (power_selected != PowerType::FIRE) {
					last_power_selected = power_selected; //lo guardamos para cuando el jugador se canse del extintor
				}
				power_selected = PowerType::FIRE; //TODO: LO LOGICO SERA ACTIVAR ESTO Y QUE PARA CAMBIAR DE ARMA HAYA QUE SELECCIONAR OTRA
				extintorMeshTimer = extintorMeshTimerDuration; //tiempo que tendra el extintor en la mano cuando deje de usarlo
				extintorActive = true;
        TCompTeleport* c_tp = get<TCompTeleport>();
        TCompTransform* c_trans = get<TCompTransform>();
        TCompMadnessController* m_c = get<TCompMadnessController>();

        if (c_tp->canCombo() && m_c->getRemainingMadness() > m_c->getPowerCost(PowerType::FIRECOMBO)) {
            if ((m_c->spendMadness(m_c->getPowerCost(PowerType::FIRECOMBO)) || GameController.getGodMode())) {//SI PUEDES HACER COMBO, Y TIENES ENERGIA
                inCombatTimer = inCombatDuration;
                c_tp->comboDone = true;
                TCompFireController* c_fire = get<TCompFireController>();
                c_fire->comboAttack(c_trans->getPosition());
            }
        }
        else if (m_c->getRemainingMadness() > m_c->getPowerCost(PowerType::FIRE) * Time.delta_unscaled) {
            if ((m_c->spendMadness(m_c->getPowerCost(PowerType::FIRE) * Time.delta_unscaled) || GameController.getGodMode()) && !c_tp->canCombo()) { // y no puedes hacer combo
        //Enable fire, keep it enabled while holding trigger, disable on release
                inCombatTimer = inCombatDuration;
                TCompFireController* c_fire = get<TCompFireController>();
                c_fire->enable();
                //Change weapon mesh 
								changeWeaponMesh(WeaponMesh::EXTINTOR);
            }
        }
    }
    else if (EngineInput["checkpoint_"].justPressed()) {
        GameController.loadCheckpoint();
    }
	//cheat(cambio de zona por posicion de mapa)
	else if (EngineInput[VK_F3].justReleased()) {
		GameController.cheatPosition();
	}

    if (power_selected == PowerType::BATTERY && inventory->getBattery() && aiming) {
        if (!isThrowingAnimationGoing && !attacking && !isCHARGED_MELEE_ATTACKGoing) {
            TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
            playerAnima->playAnimation(TCompPlayerAnimator::AIM_THROW, 1.0f);
        }
    }
	// pose apuntar player: 
	if (power_selected == PowerType::TELEPORT && inventory->getTeleport() && aiming) {
		if (!isThrowingAnimationGoing && !attacking && !isCHARGED_MELEE_ATTACKGoing) {
			TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
			playerAnima->playAnimation(TCompPlayerAnimator::SCANNER_LOOP, 1.0f);
		}
	}


    dir *= Time.delta_unscaled;
    if (aiming) {
      //Leg lookat
      //If EngineInput["front_"].value >= 0.f;
      //lookat dir
      if (EngineInput["front_"].value >= 0.f) {
        TCompSkelLookAtDirection* lookat = get<TCompSkelLookAtDirection>();
        //lookat->setDirection(dir);
      }
    }

    //MOVE PLAYER
    TCompCollider* comp_collider = get<TCompCollider>();
    if (!comp_collider || !comp_collider->controller)
        return;

    comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());

    //WHEN CHARACTER IS GROUNDED HE CAN ROTATE
    rotatePlayer(dir, Time.delta_unscaled, startDash);

    meleeTimer -= Time.delta_unscaled;

}

void TCompCharacterController::changeWeaponMesh(WeaponMesh weaponSelected) {

	CEntity* weapon = getEntityByName("Mop");
	TCompRender* w_r_mop = weapon->get<TCompRender>();
	CEntity* weapon2 = getEntityByName("Anti_extintor"); //CAMBIAR ANTIEXTINTOR POR PILA
	TCompRender* w_r_extintor = weapon2->get<TCompRender>();
	CEntity* weapon3 = getEntityByName("Scanner");
	TCompRender* w_r_scanner = weapon3->get<TCompRender>();
	CEntity* weapon4 = getEntityByName("Pila");
	TCompRender* w_r_pila = weapon4->get<TCompRender>();
    CEntity* weapon5 = getEntityByName("Energizer");
    TCompRender* w_r_energizer = weapon5->get<TCompRender>();


	if (weaponSelected == WeaponMesh::MOP) {
		w_r_mop->is_visible = true;
		w_r_extintor->is_visible = false;
		w_r_scanner->is_visible = false;
		w_r_pila->is_visible = false;
        w_r_energizer->is_visible = false;
	}
	else if (weaponSelected== WeaponMesh::SCANNER) {
		w_r_mop->is_visible = false;
		w_r_extintor->is_visible = false;
		w_r_scanner->is_visible = true;
		w_r_pila->is_visible = false;
        w_r_energizer->is_visible = false;
    }
	else if (weaponSelected == WeaponMesh::BATTERTY) {
		w_r_mop->is_visible = false;
		w_r_extintor->is_visible = false;
		w_r_scanner->is_visible = false;
		w_r_pila->is_visible = true;
        w_r_energizer->is_visible = false;
    }
	else if (weaponSelected == WeaponMesh::EXTINTOR) {
		w_r_mop->is_visible = false;
		w_r_extintor->is_visible = true;
		w_r_scanner->is_visible = false;
		w_r_pila->is_visible = false;
        w_r_energizer->is_visible = false;
    }
    else if (weaponSelected == WeaponMesh::ENERGIZER) {
        w_r_mop->is_visible = false;
        w_r_extintor->is_visible = false;
        w_r_scanner->is_visible = false;
        w_r_pila->is_visible = false;
        w_r_energizer->is_visible = true;
    }

	w_r_mop->updateRenderManager();
	w_r_extintor->updateRenderManager();
	w_r_scanner->updateRenderManager();
    w_r_pila->updateRenderManager();
    w_r_energizer->updateRenderManager();
}


void TCompCharacterController::dashing(float delta) {
    //WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
    if (!h_camera.isValid()) {
        h_camera = getEntityByName("Camera");
        return;
    }

    treatRumble(Time.delta_unscaled);

    //IMPORTANT, DASHING WE DONT APPLY GRAVITY, FOR SAKE OF AIR DASHING
    VEC3 dir = VEC3();

    //WHILE DASHING, PLAYER CANT AIM, MOVE AND SHOOT
    aiming = false;
    powerSelection();

    //MOVEMENT
    TCompTransform* c_trans = get<TCompTransform>();
    dir = c_trans->getFront() * dash_speed;
    dir *= Time.delta_unscaled;

    //MOVE PLAYER
    TCompCollider* comp_collider = get<TCompCollider>();
    if (!comp_collider || !comp_collider->controller)
        return;

    comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());

    //WHEN DASHING, PLAYER CANT ROTATE

    dash -= Time.delta_unscaled;
    if (dash <= 0.0f) {
        time_to_next_dash = time_between_dashes;
        if (isGrounded()) {
            ChangeState("GROUNDED");
        }
        else {
            ChangeState("ON_AIR");
        }
    }


}

void TCompCharacterController::onAir(float delta) {
    TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
    playerAnima->playAnimation(TCompPlayerAnimator::ON_AIR, 1.0f);
    //WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
    if (!h_camera.isValid()) {
        h_camera = getEntityByName("Camera");
        return;
    }

    if (isGrounded()) {
        ChangeState("GROUNDED");
        EngineAudio.playEvent("event:/Character/Footsteps/Landing");
        playerAnima->playAnimation(TCompPlayerAnimator::JUMP_END, 1.0f);

        return;
    }

    if (!footSteps.getPaused()) {
        footSteps.setPaused(true);
    }
    if (!footStepsSlow.getPaused()) {
        footStepsSlow.setPaused(true);
    }

    treatRumble(Time.delta_unscaled);
    bool startDash = false;

    VEC3 dir = VEC3();

    if (time_to_next_dash > 0.0f)
        time_to_next_dash -= Time.delta_unscaled;

    //ON AIR, PLAYER CAN AIM, SHOOT AND DASH, HE CANT MOVE
    powerSelection();
    if (EngineInput["aim_"].isPressed() && aiming) {//AIM
        aiming = true;
    }
    if (EngineInput["shoot_"].justPressed() && aiming) {//SHOOT
        shoot();
    }
    if (EngineInput["dash_"].justPressed() && time_to_next_dash <= 0.0f && !aiming) {//DASH
        ChangeState("DASHING");
        dash = dash_limit;
        startDash = true;
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        playerAnima->playAnimation(TCompPlayerAnimator::DASH, 1.5f);
        EngineAudio.playEvent("event:/Character/Other/Dash");
    }
    else if (EngineInput["jump_"].justPressed() && can_double_jump) { //DOUBLE JUMP
        can_double_jump = false;
        TCompRigidBody* c_rbody = get<TCompRigidBody>();
        if (!c_rbody)
            return;
        EngineAudio.playEvent("event:/Character/Footsteps/Jump_Start");
        c_rbody->doubleJump(VEC3(0.0f, double_jump_force, 0.0f));
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        playerAnima->playAnimation(TCompPlayerAnimator::DOUBLE_JUMP, 1.f);
    }
    else if (EngineInput["attack_"].justPressed() && !aiming && meleeTimer <= 0) {//ATTACK
        ChangeState("ATTACKING");
    }
    else if (EngineInput["interact_"].justPressed()) {//INTERACT
        interact();
    }
    else if (EngineInput["coffee_time_"].justPressed()) {
        //dbg("switch coffe air\n");
        TCompCoffeeController* c_coffee = get<TCompCoffeeController>();
        c_coffee->switchState();
    }
    else if (EngineInput["fire_attack_"].isPressed() /*&& unLockableChilli*/) { //FIRE
        TCompTeleport* c_tp = get<TCompTeleport>();
        TCompTransform* c_trans = get<TCompTransform>();
        TCompMadnessController* m_c = get<TCompMadnessController>();

        if ((m_c->spendMadness(m_c->getPowerCost(PowerType::FIRE) * Time.delta_unscaled) || GameController.getGodMode()) && !c_tp->canCombo()) { // y no puedes hacer combo
        //Enable fire, keep it enabled while holding trigger, disable on release
            inCombatTimer = inCombatDuration;
            TCompFireController* c_fire = get<TCompFireController>();
            c_fire->enable();
        }
        else if ((m_c->spendMadness(m_c->getPowerCost(PowerType::FIRECOMBO)) || GameController.getGodMode()) && c_tp->canCombo()) { //SI PUEDES HACER COMBO, Y TIENES ENERGIA
            inCombatTimer = inCombatDuration;
            c_tp->comboDone = true;
            TCompFireController* c_fire = get<TCompFireController>();
            c_fire->comboAttack(c_trans->getPosition());
        }
    }
	else if (EngineInput[VK_F3]) {
		GameController.cheatPosition();
	}

    getInputForce(dir);
    dir *= Time.delta_unscaled;

    //MOVE PLAYER TO THE GROUND
    TCompCollider* comp_collider = get<TCompCollider>();
    if (!comp_collider || !comp_collider->controller)
        return;

    comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());

    //WE GET THE INPUT FORCE SO WE CAN ROTATE THE PLAYER TO THAT DIRECTION, NOT MOVE!
    getInputForce(dir);

    //ON AIR, THE CHARACTER CAN MOVE OR ROTATE
    rotatePlayer(dir, Time.delta_unscaled, startDash);

    meleeTimer -= Time.delta_unscaled;
}

void TCompCharacterController::damaged(float delta) {

    if (isGrounded()) {//END RECOIL WHEN IS GROUNDED
        if (!isMounted) {
            ChangeState("GROUNDED");
        }
        else {
            ChangeState("MOUNTED");
        }
    }
}

void TCompCharacterController::dead(float delta) {
    //WE NEED THE CAMERA TO BE CREATED TO MOVE FROM ITS PERSPECTIVE
    treatRumble(Time.delta_unscaled);
    VEC3 dir = VEC3();

    //MOVE PLAYER ONLY WITH GRAVITY
    TCompCollider* comp_collider = get<TCompCollider>();
    if (!comp_collider || !comp_collider->controller)
        return;
	
    comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());

   // TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
   // playerAnima->playAnimation(TCompPlayerAnimator::DEAD, 1.f);

    //------------------
    //TMsgBlackboard msg;
    //msg.player_dead = true;
    TCompBlackboard* c_bb = get<TCompBlackboard>();
    c_bb->playerIsDeath(true);
    //------------------

    TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
    playerAnima->playAnimation(TCompPlayerAnimator::DEAD, 1.0f);


    if (EngineInput["checkpoint_"].justPressed()) {
        GameController.loadCheckpoint();
        //------------------
        ChangeState("GROUNDED");
    }
}

void TCompCharacterController::win(float delta) {
	TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
	playerAnima->playAnimation(TCompPlayerAnimator::IDLE_MELEE, 1.f, true);
    if (EngineInput["checkpoint_"].justPressed()) {
        endGame = false;
        //ChangeState("GROUNDED");

    }

}

void TCompCharacterController::noclip(float delta) {
    VEC3 dir = VEC3();
    getInputForce(dir);
    dir *= Time.delta_unscaled;
    //dir.Normalize();
    TCompCollider* comp_collider = get<TCompCollider>();
    if (!comp_collider || !comp_collider->controller)
        return;
    TCompTransform* c_trans = get<TCompTransform>();
    Vector3 currentPos = c_trans->getPosition();
    Vector3 nextPos = currentPos + (dir * speed * Time.delta_unscaled);

    PxFilterData* filterData0 = (new PxFilterData(PxFilterObjectType::eRIGID_STATIC, PxFilterObjectType::eRIGID_DYNAMIC, PxFilterObjectType::eRIGID_STATIC | PxFilterObjectType::eRIGID_DYNAMIC, PxFilterObjectType::eRIGID_DYNAMIC | PxFilterObjectType::eRIGID_DYNAMIC));

    comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters(filterData0, 0, 0));
    //comp_collider->controller->setPosition(VEC3_TO_PXEXVEC3(nextPos));

}


//UTILS

void TCompCharacterController::treatRumble(float delta) {
    rumble_time -= Time.delta_unscaled;
    if (rumble_time < 0.0f) {
        Input::TRumbleData rumble;
        rumble.leftVibration = 0.0f;
        rumble.rightVibration = 0.0f;
        EngineInput.feedback(rumble);
        rumble_time = 0.0f;
    }
}

void TCompCharacterController::getInputForce(VEC3 &dir) {
    //Movement from camera perspective
    CEntity* e_camera = (CEntity *)h_camera;
    TCompTransform* cam_trans = e_camera->get<TCompTransform>();

    VEC3 camera_left = cam_trans->getLeft();
    VEC3 camera_front = cam_trans->getFront();
    if (getState() != "NOCLIP") {
        camera_left.y = 0.0f;
        camera_front.y = 0.0f;
    }
    camera_left.Normalize();
    camera_front.Normalize();

    //KEYBOARD
    if (EngineInput["front_"].isPressed()) {
        dir += camera_front;
    }
    if (EngineInput["back_"].isPressed()) {
        dir -= camera_front;
    }
    if (EngineInput["left_"].isPressed()) {
        dir += camera_left;
    }
    if (EngineInput["right_"].isPressed()) {
        dir -= camera_left;
    }
    //GAMEPAD
    if (EngineInput.gamepad()._connected) {
        //TO CORRECT A BUG WITH GAMEPAD CONNECTED WITH KEYBOARD
        if (!EngineInput["w"].isPressed())
            dir += camera_front * EngineInput["front_"].value;

        if (!EngineInput["a"].isPressed())
            dir -= camera_left * EngineInput["left_"].value;
    }

    float length = clamp(dir.Length(), 0.f, 1.f);
    dir.Normalize();

    if (isGrounded()) {
        TCompRigidBody* r_body = get<TCompRigidBody>();
		if(r_body != nullptr) {
			if (r_body->ground_normal != VEC3::Zero) {
				VEC3 temp = r_body->ground_normal.Cross(dir);
				dir = temp.Cross(r_body->ground_normal);
			}
		}
    }

    dir *= speed * length;
    movementDirection = dir;
}

void TCompCharacterController::rotatePlayer(const VEC3 &dir, float delta, bool start_dash) {
    TCompTransform* c_trans = get<TCompTransform>();
    VEC3 player_pos = c_trans->getPosition();

    CEntity* e_camera = (CEntity *)h_camera;
    TCompTransform* cam_trans = e_camera->get<TCompTransform>();
    VEC3 camera_front = cam_trans->getFront();
    camera_front.y = 0.0f;

    VEC3 norm_dir = VEC3();
    dir.Normalize(norm_dir);

    float elapsed = 1.0f;
    if (last_frame_aiming && !aiming) {
      rotation_from_aim = 0.25f;
    }
      

    if ((dir.x != 0.0f || dir.z != 0.0f) && (!aiming || start_dash)) { //ROTATE PLAYER WHERE HE WALKS
                                                                       //And also rotate the player on the direction its facing
        float yaw, pitch;
        c_trans->getAngles(&yaw, &pitch);

        float wanted_yaw = c_trans->getDeltaYawToAimTo(player_pos + dir);

        elapsed = Time.real_scale_factor;
        if (rotation_from_aim > 0.f)
            elapsed = Time.delta_unscaled * rotation_speed * 2.0f;

        if (abs(wanted_yaw) > 0.01)
            c_trans->setRotation(QUAT::CreateFromYawPitchRoll(yaw + wanted_yaw * elapsed, pitch, 0.0f));
    }
    else if (aiming && !start_dash) { //ROTATE PLAYER WHERE THE CAMERA LOOKS
        float yaw, pitch;
        c_trans->getAngles(&yaw, &pitch);

        float wanted_yaw = c_trans->getDeltaYawToAimTo(player_pos + camera_front);

        if (!start_dash)
            elapsed = Time.delta_unscaled * rotation_speed;

        if (abs(wanted_yaw) > 0.01)
            c_trans->setRotation(QUAT::CreateFromYawPitchRoll(yaw + wanted_yaw * elapsed, pitch, 0.0f));
    }

    rotation_from_aim -= delta;
    last_frame_aiming = aiming;
}

bool TCompCharacterController::isGrounded() {
    TCompRigidBody* r_body = get<TCompRigidBody>();
	if(r_body != nullptr) {
		return r_body->isGrounded();
	}
}

void TCompCharacterController::powerSelection() {
  if (EngineInput["select_teleport_"].justPressed()) { //teleport
    power_selected = PowerType::TELEPORT; 

		changeWeaponMesh(WeaponMesh::SCANNER);
	
	
  }
  else if (EngineInput["select_battery_"].justPressed()) { //bateria
	CEntity* entity = EngineEntities.getInventoryHandle();
	TCompInventory* inventory = entity->get<TCompInventory>();
	if(inventory->getBattery()) {
		power_selected = PowerType::BATTERY;
		changeWeaponMesh(WeaponMesh::BATTERTY);
	}
  }
}

void TCompCharacterController::shoot() {
    if (EngineInput.gamepad()._connected) { //RUMBLE AT SHOOT
        Input::TRumbleData rumble;
        rumble.leftVibration = 0.2f;
        rumble.rightVibration = 0.2f;
        EngineInput.feedback(rumble);
        rumble_time = 0.3f;
    }
    inCombatTimer = inCombatDuration;

    TCompTransform* c_trans = get<TCompTransform>();
    TCompMadnessController* m_c = get<TCompMadnessController>();

	CEntity* entity = EngineEntities.getInventoryHandle();
	TCompInventory* inventory = entity->get<TCompInventory>();


    if (power_selected == PowerType::TELEPORT && (inventory->getTeleport())) {
        //If we have enough madness, we can use the power
        if (m_c->spendMadness(PowerType::TELEPORT) || GameController.getGodMode()) {
            TCompTeleport* c_tp = get<TCompTeleport>();
            c_tp->rayCast();
            //Change weapon mesh
						changeWeaponMesh(WeaponMesh::SCANNER);
            //Execute animation
            TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
            playerAnima->playAnimation(TCompPlayerAnimator::SCAN, 1.f, true);
        }
    }
    else if (power_selected == PowerType::BATTERY && (inventory->getBattery())) {
        if (isBatteryAlive)
            return;
        if (m_c->spendMadness(PowerType::BATTERY) || GameController.getGodMode()) {
            TCompTransform* c_trans = get<TCompTransform>();
            CEntity* e_camera = (CEntity *)h_camera;
            TCompTransform* comp_cam = e_camera->get<TCompTransform>();
            VEC3 far_cam_target = (comp_cam->getFront() * distance_to_aim) + comp_cam->getPosition(); //DISTANCE WHERE IS POINTING
            VEC3 front = far_cam_target - c_trans->getPosition();
            front.Normalize();
            TCompBatteryController* c_bat = get<TCompBatteryController>();
            c_bat->shoot(front);
            aiming = false;
            TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
            playerAnima->playAnimation(TCompPlayerAnimator::THROW, 0.5f, true);
            isThrowingAnimationGoing = true;
            EngineAudio.playEvent("event:/Character/Powers/Battery/Throw");
            isBatteryAlive = true;
        }
    }
}

void TCompCharacterController::attack(float delta) {
    //Change weapon mesh
		changeWeaponMesh(WeaponMesh::MOP);
    //COJO EL COMPONENTE, SI PUEDO HACER COMBO, PUES OTRA ANIMACION Y LA FUERZA ES MAYOR

    inCombatTimer = inCombatDuration;
    VEC3 dir = VEC3();
    getInputForce(dir);
    dir *= Time.delta_unscaled;
    TCompCollider* comp_collider = get<TCompCollider>();
    if (!comp_collider || !comp_collider->controller)
        return;

    if (!attacking) {
        attacking = true;

        TCompTransform* c_trans = get<TCompTransform>();
        //Create a collider sphere where we want to detect collision
        PxSphereGeometry geometry(meleeRadius);
        Vector3 damageOrigin = c_trans->getPosition() + (c_trans->getFront() * meleeDistance);
        PxF32 attackHeight = comp_collider->controller->getHeight();
        damageOrigin.y = c_trans->getPosition().y + (float)attackHeight - 0.5f;
        PxVec3 pos = VEC3_TO_PXVEC3(damageOrigin);
        PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());

        const PxU32 bufferSize = 256;
        PxOverlapHit hitBuffer[bufferSize];
        PxOverlapBuffer buf(hitBuffer, bufferSize);
        PxTransform shapePose = PxTransform(pos, ori);
        PxQueryFilterData filter_data = PxQueryFilterData();
        filter_data.data.word0 = EnginePhysics.Puddle | EnginePhysics.Product;
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
              msg.impactForce = impactForceAttack * comboModifier;
              msg.damageType = MELEE;
              msg.targetType = ENEMIES;
              entityContact->sendMsg(msg);

              alreadyAttacked = true;
              meleeHit = true;
            }
          }
          if (meleeHit) {
            EngineAudio.playEvent("event:/Character/Attacks/Melee_Hit");

            CEntity* onom_manager = getEntityByName("Onomatopoeia Particles");
            TMsgOnomPet msgonom;
            msgonom.type = 1.0f;
            msgonom.pos = c_trans->getPosition();
            onom_manager->sendMsg(msgonom);
          }
        }

        //Execute animation
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        if (animation1Done) {
            playerAnima->playAnimation(TCompPlayerAnimator::MELEE2_PARTIAL, 1.f, true);
            animation1Done = false;
        }
        else {
            playerAnima->playAnimation(TCompPlayerAnimator::MELEE1_PARTIAL, 0.6f, true);
            animation1Done = true;
        }
        
        EngineAudio.playEvent("event:/Character/Attacks/Melee_Swing");
    }

    TCompTeleport* c_tp = get<TCompTeleport>();
    TCompTransform* c_trans = get<TCompTransform>();
    TCompMadnessController* m_c = get<TCompMadnessController>();

    if (c_tp->canCombo()) { //puedes hacer combo
        comboModifier = 2.f;
        c_tp->comboDone = true;
    }
    else {
      comboModifier = 1.0f;
    }
    ChangeState("GROUNDED");

    comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());
    rotatePlayer(dir, Time.delta_unscaled, false);
}

void TCompCharacterController::chargedAttack(float delta) {
    //If the button is released
    if (EngineInput["attack_"].justReleased()) {
        //If the chargedAttack_buttonPressTimer is greater than chargedAttack_chargeDelay, launch the attack
        if (chargedAttack_buttonPressTimer >= chargedAttack_chargeDelay) {
            dbg("Player executes CHARGED_ATTACK\n");
            TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
            isCHARGED_MELEE_ATTACKGoing = true;
            playerAnima->playAnimation(TCompPlayerAnimator::CHARGED_MELEE_ATTACK, 0.8f, true);
            //Execute animation, should have root motion and deal the damage the moment it connects with the ground
            TCompRigidBody* c_rbody = get<TCompRigidBody>();
            if (!c_rbody)
                return;
            EngineAudio.playEvent("event:/Character/Attacks/ChargedAttack");
            //c_rbody->jump(VEC3(0.0f, jump_force, 0.0f));
            inCombatTimer = inCombatDuration;
            TCompTransform* c_trans = get<TCompTransform>();
            TMsgDamage msg;
            msg.h_bullet = CHandle(this).getOwner();
            msg.senderType = EntityType::PLAYER;
            msg.targetType = EntityType::ENEMIES;
            msg.damageType = PowerType::CHARGED_ATTACK;
            msg.position = c_trans->getPosition() + VEC3::Up;
            msg.intensityDamage = chargedAttack_damage;
			msg.impactForce = chargedAttack_impactForce;
            GameController.generateDamageSphere(c_trans->getPosition(), chargedAttack_radius, msg, "enemy");
            GameController.generateDamageSphere(c_trans->getPosition() + 1.5f * c_trans->getFront(), chargedAttack_radius, msg, "enemy");
            GameController.spawnPrefab("data/prefabs/props/explosion_soja.json", c_trans->getPosition(), c_trans->getRotation(), 2.f);
            //stop charging
            chargedAttack_buttonPressTimer = 0.f;
            dbg("Player lands CHARGED_ATTACK.\n");
            ChangeState("GROUNDED");
            return;
        }

        //If we stop holding before the attack is charged, go back to grounded
        if (EngineInput["attack_"].justReleased() && chargedAttack_buttonPressTimer < chargedAttack_chargeDelay) {
            //stop charging
            chargedAttack_buttonPressTimer = 0.f;
            dbg("Player stops charging CHARGED_ATTACK.\n");
            ChangeState("GROUNDED");
        }
    }

    //If the button is pressed increase chargedAttack_buttonPressTimer
    if (EngineInput["attack_"].isPressed()) {
        chargedAttack_buttonPressTimer += Time.delta_unscaled;
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        playerAnima->playAnimation(TCompPlayerAnimator::CHARGED_MELEE_LOOP, 1.0f);
    }

    /*VEC3 dir = VEC3();
    getInputForce(dir);
    dir *= Time.delta_unscaled;
    TCompCollider* comp_collider = get<TCompCollider>();
    if (!comp_collider || !comp_collider->controller)
        return;

    comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());
    rotatePlayer(dir, Time.delta_unscaled, false);*/
}

//void TCompCharacterController::attackChilli(float delta) {
//  VEC3 dir = VEC3();
//  getInputForce(dir);
//  dir *= Time.delta_unscaled;
//  TCompCollider* comp_collider = get<TCompCollider>();
//  if (!comp_collider || !comp_collider->controller)
//    return;
//
//  if (meleeCurrentDuration <= meleeTotalDuration) {
//    //SwapMesh(4);
//    meleeCurrentDuration += Time.delta_unscaled;
//  }
//  else {
//    meleeCurrentDuration = 0.f;
//    meleeTimer = meleeDelay;
//    ChangeState("GROUNDED");
//
//    TCompFireController* c_fire = get<TCompFireController>();
//    c_fire->inUse = false;
//  }
//
//  comp_collider->controller->move(VEC3_TO_PXVEC3(dir), 0.0f, Time.delta_unscaled, PxControllerFilters());
//  rotatePlayer(dir, Time.delta_unscaled, false);
//}

//EVENTS

void TCompCharacterController::onCollision(const TMsgOnContact& msg) {
    CEntity* source_of_impact = (CEntity *)msg.source.getOwner();
    if (!source_of_impact)
      return;
    TCompDynamicInstance* dyn_ = source_of_impact->get<TCompDynamicInstance>();
    if (dyn_) {
      TCompTransform* c_trans = get<TCompTransform>();
      TMsgDamage msg;
      // Who sent this bullet
      msg.h_sender = CHandle(this).getOwner();
      msg.h_bullet = CHandle(this).getOwner();
      msg.position = c_trans->getPosition() + VEC3::Up;
      msg.senderType = PLAYER;
      msg.intensityDamage = movementDirection.Length();
      msg.impactForce = movementDirection.Length();
      msg.damageType = MELEE;
      msg.targetType = ENEMIES;
      source_of_impact->sendMsg(msg);
    }
}

void TCompCharacterController::onEnter(const TMsgEntityTriggerEnter& trigger_enter) {
    CEntity* e = (CEntity *)trigger_enter.h_entity;

    if (e) {
        TCompTags* c_tag = e->get<TCompTags>();
        if (c_tag) {
            std::string tag = CTagsManager::get().getTagName(c_tag->tags[0]);
            if (strcmp("checkpoint", tag.c_str()) == 0) {
                TCompTransform* transfCkeckpoint = e->get<TCompTransform>();
                posCkeckpoint = transfCkeckpoint->getPosition();
            }
            else if (strcmp("endgame", tag.c_str()) == 0) {
                //..en el futuro GameState GameOver
                //endGame = true;
				GameController.playAnnouncement("event:/UI/Announcements/Announcement12");
                //dismount();
                ChangeState("WIN");
				CEngine::get().getUI().deactivateWidgetClass("HUD_NORMAL_PLAYER");
				CEngine::get().getUI().deactivateWidgetClass("MISION_9");
				CEngine::get().getUI().deactivateWidgetClass("MISION_8");
				CEngine::get().getUI().deactivateWidgetClass("MISION_7");
				CEngine::get().getUI().deactivateWidgetClass("MISION_6");
				CEngine::get().getUI().deactivateWidgetClass("MISION_5");
				CEngine::get().getUI().deactivateWidgetClass("MISION_4");
				CEngine::get().getUI().deactivateWidgetClass("MISION_3");
				CEngine::get().getUI().deactivateWidgetClass("MISION_2");
				CEngine::get().getUI().deactivateWidgetClass("MISION_1");
				Scripting.execActionDelayed("endGame()", 0.0);
				

            }
            //else if(){} demas trigers...
        }
    }
}

void TCompCharacterController::onDamageAll(const TMsgDamageToAll& msg) {
    if (!GameController.getGodMode() && !cinematic && invulnerabilityTimer <= 0 && !GameController.getResurrect()) {
        life -= msg.intensityDamage;
        damagedAudio = EngineAudio.playEvent("event:/Character/Voice/Player_Pain");

        TCompTransform* c_trans = get<TCompTransform>();
        CEntity* onom_manager = getEntityByName("Onomatopoeia Particles");
        TMsgOnomPet msgonom;
        msgonom.type = 7.0f;
        msgonom.pos = c_trans->getPosition();
        onom_manager->sendMsg(msgonom);


        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        playerAnima->playAnimation(TCompPlayerAnimator::DAMAGED, 1.f, false);
		    invulnerabilityTimer = invulnerabilityTimeDuration;
        inCombatTimer = inCombatDuration;
    }


    if (life <= 0.0f) {
        life = 0.0f;
        ChangeState("DEAD");
        TCompSCartController* sCart = get<TCompSCartController>();
        sCart->disable();
    }
    else {

        ChangeState("DAMAGED");

        TCompRigidBody* c_rbody = get<TCompRigidBody>();
        if (!c_rbody)
            return;
        c_rbody->addForce(VEC3(0, 0, 0));
    }

}

void TCompCharacterController::onTrapWind(const TMsgTrapWind& msg) {
  //dbg("recibo damage \n");
  if (!GameController.getGodMode() && !cinematic) {
    if (strcmp("DAMAGED", state.c_str()) != 0 && msg.targetType == EntityType::PLAYER || msg.targetType == EntityType::ALL) {
      life -= msg.intensityDamage;
      TCompTransform* my_trans = get<TCompTransform>();
      VEC3 direction_to_damage;
      if (msg.senderType == ENEMIES) { //los enemigos envian el handle
        CEntity* entity_to_hit_me = (CEntity *)msg.h_bullet;
        TCompTransform* e_trans = entity_to_hit_me->get<TCompTransform>();
        direction_to_damage = my_trans->getPosition() - e_trans->getPosition();
      }
      else { //algunos objetos envian una posicion
        direction_to_damage = my_trans->getPosition() - msg.position;
      }
      direction_to_damage.Normalize();

      TCompRigidBody* c_rbody = get<TCompRigidBody>();
      if (c_rbody && strcmp("CHARGED_ATTACK", state.c_str()) != 0)
        c_rbody->addForce(direction_to_damage * msg.impactForce);

      if (life <= 0.0f) {
        life = 0.0f;
        EngineAudio.playEvent("event:/Character/Voice/Player_Death");
        footSteps.setPaused(true);
        footStepsSlow.setPaused(true);
        ChangeState("DEAD");
        TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
        playerAnima->playAnimation(TCompPlayerAnimator::DIE, 0.5f, true);
        TCompSCartController* sCart = get<TCompSCartController>();
        sCart->disable();
      }
      else {

        if (msg.senderType == ENEMIES) {
          //	c_rbody->addForce(direction_to_damage * 8.0f);
        }
        if (&(msg.impactForce) != nullptr && msg.impactForce > 0) {
          TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
          playerAnima->playAnimation(TCompPlayerAnimator::DAMAGED, 1.0f);
        }
        if (&(msg.impactForce) != nullptr && msg.impactForce > 0 && !damagedAudio.isPlaying()) {
            damagedAudio = EngineAudio.playEvent("event:/Character/Voice/Player_Pain");

            TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
            playerAnima->playAnimation(TCompPlayerAnimator::DAMAGED, 1.f, false);
        }
      }
    }
  }
}

void TCompCharacterController::onGenericDamage(const TMsgDamage& msg) {

    if (life <= 0.0f) {
        return;
    }
    //dbg("recibo damage \n");
    if (!GameController.getGodMode() && !cinematic && invulnerabilityTimer <= 0 && !GameController.getResurrect()) {
        if (strcmp("DAMAGED", state.c_str()) != 0 && msg.targetType == EntityType::PLAYER || msg.targetType == EntityType::ALL) {
            life -= msg.intensityDamage;
			      invulnerabilityTimer = invulnerabilityTimeDuration;
            inCombatTimer = inCombatDuration;
            CEntity* e_cam = getEntityByName("MainCamera");
            TMsgOnContact msg_cam;
            e_cam->sendMsg(msg_cam);
                        
            TCompTransform* my_trans = get<TCompTransform>();
            VEC3 direction_to_damage;
            if (msg.senderType == ENEMIES) { //los enemigos envian el handle
                CEntity* entity_to_hit_me = (CEntity *)msg.h_bullet;
                TCompTransform* e_trans = entity_to_hit_me->get<TCompTransform>();
                direction_to_damage = my_trans->getPosition() - e_trans->getPosition();
            }
            else { //algunos objetos envian una posicion
                direction_to_damage = my_trans->getPosition() - msg.position;
            }
            direction_to_damage.Normalize();

            TCompRigidBody* c_rbody = get<TCompRigidBody>();
            if (c_rbody && strcmp("CHARGED_ATTACK", state.c_str()) != 0)
                c_rbody->addForce(direction_to_damage * msg.impactForce);

            if (life <= 0.0f) {
                life = 0.0f;
                EngineAudio.playEvent("event:/Character/Voice/Player_Death");
                footSteps.setPaused(true);
                footStepsSlow.setPaused(true);
                ChangeState("DEAD");
                TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
                playerAnima->playAnimation(TCompPlayerAnimator::DIE, 0.5f, true);

                CEntity* e_carrito = getEntityByName("Carrito");
                TCompRender* r_carrito = e_carrito->get<TCompRender>();
                r_carrito->is_visible = false;
                r_carrito->updateRenderManager();
            }
            else {

                if (msg.senderType == ENEMIES) {
                    //	c_rbody->addForce(direction_to_damage * 8.0f);
                }
                if (&(msg.impactForce) != nullptr && msg.intensityDamage > 0 && !damagedAudio.isPlaying()) {
                    damagedAudio = EngineAudio.playEvent("event:/Character/Voice/Player_Pain");

                    TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
                    playerAnima->playAnimation(TCompPlayerAnimator::DAMAGED, 1.f, false);
                    

                    TCompTransform* c_trans = get<TCompTransform>();
                    CEntity* onom_manager = getEntityByName("Onomatopoeia Particles");
                    TMsgOnomPet msgonom;
                    msgonom.type = 7.0f;
                    msgonom.pos = c_trans->getPosition();
                    onom_manager->sendMsg(msgonom);
                }
            }
        }
    }
}

void TCompCharacterController::onTriggerFalloutDead(const TMSgTriggerFalloutDead& msg) {
	
	if (!GameController.getGodMode() && !cinematic) {
		life -= msg.damage;
		invulnerabilityTimer = invulnerabilityTimeDuration;
	}


	if (life <= 0.0f) {
		life = 0.0f;
		ChangeState("DEAD");
    TCompSCartController* sCart = get<TCompSCartController>();
    sCart->disable();
	}
	else {

		ChangeState("DAMAGED");

		TCompRigidBody* c_rbody = get<TCompRigidBody>();
		if (!c_rbody)
			return;
		c_rbody->addForce(VEC3(0, 0, 0));
	}


}


void TCompCharacterController::onPowerUp(const TMsgPowerUp& msg) {

}

void TCompCharacterController::onCinematic(const TMsgOnCinematic & msg)
{
	if(!msg.isscart){
		
		if(msg.cinematic){
			TCompPlayerAnimator* playerAnima = get<TCompPlayerAnimator>();
			playerAnima->playAnimation(TCompPlayerAnimator::IDLE_MELEE, 1.f, true);
			ChangeState("IDLE_CINEMATIC");
		}
		cinematic = msg.cinematic;
		UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
		mirilla->getParams()->visible = false;
	}
	else {
		cinematic = msg.cinematic;
		UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
		mirilla->getParams()->visible = false;
	}
}

void TCompCharacterController::onCinematicSpecial(const TMsgOnCinematicSpecial & msg)
{
	if (!msg.isscart) {

		if (msg.cinematic) {
			if (msg.type == 1) {//cambiar el por DEFINE
				TCompTransform* c_trans = get<TCompTransform>();
				
				c_trans->rotateTowards(targetTower);
				ChangeState("ESPECIAL_CINEMATIC");
			}
		}
		cinematic = msg.cinematic;
		UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
		mirilla->getParams()->visible = false;
	}
	else {
		cinematic = msg.cinematic;
		UI::CImage* mirilla = dynamic_cast<UI::CImage*>(Engine.getUI().getWidgetByAlias("reticula_"));
		mirilla->getParams()->visible = false;
		TCompSCartController* sCart = get<TCompSCartController>();
		sCart->disable();
		TCompTransform* c_trans = get<TCompTransform>();
		c_trans->rotateTowards(targetTower);

		ChangeState("ESPECIAL_CINEMATIC");
		//dbg("Player changes to MOUNTED\n");
		
	}
}





void TCompCharacterController::onBatteryDeactivation(const TMsgBatteryDeactivates& msg) {
    isBatteryAlive = false;
}


//OTHERS

void  TCompCharacterController::SwapMesh(int state) {
    TCompRender* crender = get<TCompRender>();
    crender->showMeshesWithState(state);
}

//Shopping Cart
void TCompCharacterController::mount(CHandle vehicle) {
    if (vehicle.isValid()) {
        isMounted = true;
        TCompSCartController* sCart = get<TCompSCartController>();
        sCart->enable(vehicle);
        //dbg("Player changes to MOUNTED\n");
        ChangeState("MOUNTED");
        //SwapMesh(1);
        footSteps.setPaused(true);
        footStepsSlow.setPaused(true);
        changeWeaponMesh(WeaponMesh::MOP);
    }
}

void TCompCharacterController::dismount() {
    //dbg("Player DISMOUNTS\n");
    isMounted = false;
    //While moving appear behind sCart
    //While stationary appear in front of sCart
    //SwapMesh(0);
}

void TCompCharacterController::mounted(float delta) {
    if (!isMounted) {
        //dbg("Player changes to GROUNDED from MOUNTED\n");
        ChangeState("GROUNDED");
    }
}

void TCompCharacterController::interact() {
    //Scripting.execAction("playAnnouncement('event:/UI/Announcements/Announcement1')");
    TCompTransform* c_trans = get<TCompTransform>();
    //Analyze interactable objects
    //Create a collider sphere where we want to detect collision
    PxSphereGeometry geometry(interactRange * 2);
    PxVec3 pos = VEC3_TO_PXVEC3(c_trans->getPosition());
    PxQuat ori = QUAT_TO_PXQUAT(c_trans->getRotation());

    const PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
    PxOverlapHit hitBuffer[bufferSize];  // [out] User provided buffer for results
    PxOverlapBuffer buf(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here

    PxTransform shapePose = PxTransform(pos, ori);    // [in] initial shape pose (at distance=0)
                                                      //Evaluate collisions

    physx::PxFilterData pxFilterData;
    pxFilterData.word0 = EnginePhysics.Carrito;
    physx::PxQueryFilterData PxPlayerFilterData;
    PxPlayerFilterData.data = pxFilterData;
    bool res = EnginePhysics.gScene->overlap(geometry, shapePose, buf, PxPlayerFilterData);

    //If there were any hits
    if (res) {
        //Analyze entities hit
        for (PxU32 i = 0; i < buf.nbTouches; i++) {
            CHandle h_comp_physics;
            h_comp_physics.fromVoidPtr(buf.getAnyHit(i).actor->userData);
            CEntity* entityContact = h_comp_physics.getOwner();
            if (entityContact) {
                dbg("Pete con carrito: %s\n", entityContact->getName());
                mount(h_comp_physics);
            }
        }
    }
}

void TCompCharacterController::setMeleeMultiplier(float newMulti) {
    meleeDamage *= newMulti;
}

void TCompCharacterController::setSpeed(float newSpeed) {
    speed = newSpeed;
}

float TCompCharacterController::getBaseSpeed() {
    return base_speed;
}

void  TCompCharacterController::heal() {
    life = maxLife;
}

void  TCompCharacterController::healPartially(float health) {
    life += health;
    if (life > maxLife)
        life = maxLife;
}

void TCompCharacterController::restoreMadness() {
    TCompMadnessController* madness = get<TCompMadnessController>();
    madness->restoreMadness();
}

float TCompCharacterController::getMaxMadness() {
    TCompMadnessController* madness = get<TCompMadnessController>();
    return madness->getMaximumMadness();
}


void  TCompCharacterController::applyPowerUp(float quantity, PowerUpType type, float extraBarSize) {

    switch (type) {
      case PowerUpType::HEALTH_UP:
      {
          //dbg("aplica el power up de life \n");
          //maxLife = maxLife + quantity;
          //heal();
          //GameController.increaseHpBarSize(extraBarSize);
					GameController.healPlayerPartially(quantity);
          EngineAudio.playEvent("event:/Character/Other/Powerup_Pickup");
          break;
      }
      case PowerUpType::MADNESS_UP:
      {
          //dbg("aplica el power up de la locura \n");
          TCompMadnessController* madness = get<TCompMadnessController>();
          madness->setMaximumMadness(madness->getMaximumMadness() + quantity);
          restoreMadness();
          GameController.increaseMadnessBarSize(extraBarSize);
          EngineAudio.playEvent("event:/Character/Other/Powerup_Pickup");
          break;
      }
      case PowerUpType::ACTIVATE_BATTERY:
      {
          //TODO
			CEntity* entity = EngineEntities.getInventoryHandle();
			TCompInventory* inventory = entity->get<TCompInventory>();
			inventory->setBattery(true);

			//llamada funcion de scripting para poder escapar
			//Scripting.execActionDelayed("activarSalidaPanaderia()", 0.0);
			  
			//Scripting.execActionDelayed("saveCheckpoint()", 20.0);

			Scripting.execActionDelayed("crearTrampaHornos()", 0.0);
			Scripting.execActionDelayed("childAppears(\"MISION_2\",true,true,0.0,1.25)",0.1);
			//Scripting.execActionDelayed("deactivateWidget(\"MISION_1\")", 0.0);
			Scripting.execActionDelayed("saveCheckpoint()", 0.5);

			/*
			CEntity* e = GameController.entityByName("horno008");
			TCompEnemySpawner* tComp = e->get<TCompEnemySpawner>();
			CHandle h(tComp);
			h.destroy();
			*/

      CEntity* e1 = getEntityByName("Hielo2_LP");
      TCompMorphAnimation* c_ma1 = e1->get<TCompMorphAnimation>();
      c_ma1->updateMorphData(0.0f);
      CEntity* e2 = getEntityByName("Hielo5_LP");
      TCompMorphAnimation* c_ma2 = e2->get<TCompMorphAnimation>();
      c_ma2->updateMorphData(0.0f);
      CEntity* e3 = getEntityByName("Hielo6_LP");
      TCompMorphAnimation* c_ma3 = e3->get<TCompMorphAnimation>();
      c_ma3->updateMorphData(0.0f);
      CEntity* e4 = getEntityByName("cubosHielo_033");

      TCompMorphAnimation* c_ma4 = e4->get<TCompMorphAnimation>();
      c_ma4->updateMorphData(0.0f);

      CEntity* e5 = getEntityByName("cubosHielo_034");

      TCompMorphAnimation* c_ma5 = e5->get<TCompMorphAnimation>();
      c_ma5->updateMorphData(0.0f);


			EngineAudio.playEvent("event:/Character/Other/Weapon_Pickup");
			break;
      }
      case PowerUpType::ACTIVATE_CHILLI:
      {
          //TODO
		      CEntity* entity = EngineEntities.getInventoryHandle();
		      TCompInventory* inventory = entity->get<TCompInventory>();
		      inventory->setChilli(true);
		      EngineAudio.playEvent("event:/Character/Other/Weapon_Pickup");
		      Scripting.execActionDelayed("playAnnouncement(\"event:/UI/Announcements/Announcement5\")", 1.0);
		      Scripting.execActionDelayed("childAppears(\"MISION_4\",true,true,0.0,1.25)", 1.1);
			  Scripting.execActionDelayed("saveCheckpoint()", 0);
		  
          break;
      }
      case PowerUpType::ACTIVATE_COFFEE:
      {
		    CEntity* entity = EngineEntities.getInventoryHandle();
		    TCompInventory* inventory = entity->get<TCompInventory>();
		    inventory->setCoffe(true);
        EngineAudio.playEvent("event:/Character/Other/Weapon_Pickup");
		    Scripting.execActionDelayed("childAppears(\"MISION_9\",true,true,0.0,1.25)", 0);
        break;
      }
      case PowerUpType::ACTIVATE_TELEPORT:
      {

		  //unLockableTeleport = true;
		  CEntity* entity = EngineEntities.getInventoryHandle();
		  TCompInventory* inventory = entity->get<TCompInventory>();
		  inventory->setTeleport(true);
          EngineAudio.playEvent("event:/Character/Other/Weapon_Pickup");
          break;
      }
    }

}
