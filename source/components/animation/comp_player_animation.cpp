#include "mcv_platform.h"
#include "comp_player_animation.h"
#include "components/common/comp_transform.h"


DECL_OBJ_MANAGER("player_animator", TCompPlayerAnimator);

void TCompPlayerAnimator::debugInMenu() {

    static float delta_movement = 0.0f;
    static float speed = 1.0f;
    ImGui::DragFloat("Speed", &speed, 0.01f, 0, 3.f);
    if (ImGui::SmallButton("IDLE_MELEE")) {
        playAnimation(EAnimation::IDLE_MELEE, speed);
    }
    if (ImGui::SmallButton("IDLE_FIRE")) {
        playAnimation(EAnimation::IDLE_FIRE, speed);
    }
    if (ImGui::SmallButton("IDLE_COMBAT")) {
        playAnimation(EAnimation::IDLE_COMBAT, speed);
    }
    if (ImGui::SmallButton("MELEE1_FULL")) {
        playAnimation(EAnimation::MELEE1_FULL, speed);
    }
    if (ImGui::SmallButton("MELEE1_PARTIAL")) {
        playAnimation(EAnimation::MELEE1_PARTIAL, speed);
    }
    if (ImGui::SmallButton("MELEE2_FULL")) {
        playAnimation(EAnimation::MELEE2_FULL, speed);
    }
    if (ImGui::SmallButton("MELEE2_PARTIAL")) {
        playAnimation(EAnimation::MELEE2_PARTIAL, speed);
    }
    if (ImGui::SmallButton("AIM_THROW")) {
        playAnimation(EAnimation::AIM_THROW, speed);
    }
    if (ImGui::SmallButton("DAMAGED")) {
        playAnimation(EAnimation::DAMAGED, speed);
    }
    if (ImGui::SmallButton("DASH")) {
        playAnimation(EAnimation::DASH, speed);
    }
    if (ImGui::SmallButton("DOUBLE_JUMP")) {
        playAnimation(EAnimation::DOUBLE_JUMP, speed);
    }
    if (ImGui::SmallButton("DRINK")) {
        playAnimation(EAnimation::DRINK, speed);
    }
    if (ImGui::SmallButton("JUMP_END")) {
        playAnimation(EAnimation::JUMP_END, speed);
    }
    if (ImGui::SmallButton("JUMP_START")) {
        playAnimation(EAnimation::JUMP_START, speed);
    }
    if (ImGui::SmallButton("ON_AIR")) {
        playAnimation(EAnimation::ON_AIR, speed);
    }
    if (ImGui::SmallButton("RUN")) {
        playAnimation(EAnimation::RUN, speed);
    }
    if (ImGui::SmallButton("SCAN")) {
        playAnimation(EAnimation::SCAN, speed);
    }
    if (ImGui::SmallButton("THROW")) {
        playAnimation(EAnimation::THROW, speed);
    }
    if (ImGui::SmallButton("WALK")) {
        playAnimation(EAnimation::WALK, speed);
    }
    if (ImGui::SmallButton("DEAD")) {
      playAnimation(EAnimation::DEAD, speed);
    }
    if (ImGui::SmallButton("DIE")) {
      playAnimation(EAnimation::DIE, speed);
    }
    if (ImGui::SmallButton("FIRE_END")) {
      playAnimation(EAnimation::FIRE_END, speed);
    }
    if (ImGui::SmallButton("FIRE_END_PARTIAL")) {
      playAnimation(EAnimation::FIRE_END_PARTIAL, speed);
    }
    if (ImGui::SmallButton("FIRE_LOOP")) {
      playAnimation(EAnimation::FIRE_LOOP, speed);
    }
    if (ImGui::SmallButton("FIRE_LOOP_PARTIAL")) {
      playAnimation(EAnimation::FIRE_LOOP_PARTIAL, speed);
    }
    if (ImGui::SmallButton("FIRE_START")) {
      playAnimation(EAnimation::FIRE_START, speed);
    }
    if (ImGui::SmallButton("FIRE_START_PARTIAL")) {
      playAnimation(EAnimation::FIRE_START_PARTIAL, speed);
    }
    if (ImGui::SmallButton("CHARGED_MELEE_ATTACK")) {
        playAnimation(EAnimation::CHARGED_MELEE_ATTACK, speed);
    }
    if (ImGui::SmallButton("CHARGED_MELEE_LOOP")) {
        playAnimation(EAnimation::CHARGED_MELEE_LOOP, speed);
    }
    if (ImGui::SmallButton("CHARGED_MELEE_POSE")) {
        playAnimation(EAnimation::CHARGED_MELEE_POSE, speed);
    }
    if (ImGui::SmallButton("SCART_IDLE")) {
        playAnimation(EAnimation::SCART_IDLE, speed);
    }
    if (ImGui::SmallButton("SCART_LEFT_END")) {
        playAnimation(EAnimation::SCART_LEFT_END, speed);
    }
    if (ImGui::SmallButton("SCART_LEFT_LOOP")) {
        playAnimation(EAnimation::SCART_LEFT_LOOP, speed);
    }
    if (ImGui::SmallButton("SCART_LEFT_START")) {
        playAnimation(EAnimation::SCART_LEFT_START, speed);
    }
    if (ImGui::SmallButton("SCART_RIGHT_END")) {
        playAnimation(EAnimation::SCART_RIGHT_END, speed);
    }
    if (ImGui::SmallButton("SCART_RIGHT_LOOP")) {
        playAnimation(EAnimation::SCART_RIGHT_LOOP, speed);
    }
    if (ImGui::SmallButton("SCART_RIGHT_START")) {
        playAnimation(EAnimation::SCART_RIGHT_START, speed);
    }
    if (ImGui::SmallButton("SCART_ROW")) {
        playAnimation(EAnimation::SCART_ROW, speed);
    }
    if (ImGui::SmallButton("SCANNER_LOOP")) {
        playAnimation(EAnimation::SCANNER_LOOP, speed);
    }


    ImGui::DragFloat("Delta Movement", &delta_movement, 0.01f, 0, 1.f);
    TCompSkeleton* compSkeleton = get<TCompSkeleton>();
    compSkeleton->setCyclicAnimationWeight(delta_movement);
}

void TCompPlayerAnimator::initializeAnimations() {

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::IDLE_MELEE,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "IDLE_MELEE",
        "",
        "",
        0.1f,
        1.0f
    );
    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::IDLE_COMBAT,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "IDLE_COMBAT",
        "",
        "",
        0.5f,
        1.0f
    );


    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::IDLE_FIRE,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "IDLE_FIRE",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::MELEE1_FULL,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "MELEE1_FULL",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::MELEE1_PARTIAL,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "MELEE1_PARTIAL",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::MELEE2_FULL,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "MELEE2_FULL",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::MELEE2_PARTIAL,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "MELEE2_PARTIAL",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::AIM_THROW,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "AIM_THROW",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DAMAGED,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "DAMAGED",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DASH,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "DASH",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DOUBLE_JUMP,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "DOUBLE_JUMP",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DRINK,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "DRINK",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMP_END,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "JUMP_END",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMP_START,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "JUMP_START",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::ON_AIR,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "ON_AIR",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::RUN,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "RUN",
        "",
        "",
        0.15f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCAN,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "SCAN",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::THROW,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "THROW",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::WALK,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "WALK",
        "",
        "",
        0.5f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DEAD,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "DEAD",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DIE,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "DIE",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
      (TCompAnimator::EAnimation)EAnimation::FIRE_END,
      EAnimationType::ACTION,
      EAnimationSize::SINGLE,
      "FIRE_END",
      "",
      "",
      1.0f,
      1.0f
    );

    initializeAnimation(
      (TCompAnimator::EAnimation)EAnimation::FIRE_END_PARTIAL,
      EAnimationType::ACTION,
      EAnimationSize::SINGLE,
      "FIRE_END_PARTIAL",
      "",
      "",
      1.0f,
      1.0f
    );

    initializeAnimation(
      (TCompAnimator::EAnimation)EAnimation::FIRE_LOOP,
      EAnimationType::ACTION,
      EAnimationSize::SINGLE,
      "FIRE_LOOP",
      "",
      "",
      1.0f,
      1.0f
    );

    initializeAnimation(
      (TCompAnimator::EAnimation)EAnimation::FIRE_LOOP_PARTIAL,
      EAnimationType::ACTION,
      EAnimationSize::SINGLE,
      "FIRE_LOOP_PARTIAL",
      "",
      "",
      1.0f,
      1.0f
    );

    initializeAnimation(
      (TCompAnimator::EAnimation)EAnimation::FIRE_START,
      EAnimationType::ACTION,
      EAnimationSize::SINGLE,
      "FIRE_START",
      "",
      "",
      1.0f,
      1.0f
    );

    initializeAnimation(
      (TCompAnimator::EAnimation)EAnimation::FIRE_START_PARTIAL,
      EAnimationType::ACTION,
      EAnimationSize::SINGLE,
      "FIRE_START_PARTIAL",
      "",
      "",
      1.0f,
      1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::CHARGED_MELEE_ATTACK,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "CHARGED_MELEE_ATTACK",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::CHARGED_MELEE_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "CHARGED_MELEE_LOOP",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::CHARGED_MELEE_POSE,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "CHARGED_MELEE_POSE",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_IDLE,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "SCART_IDLE",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_LEFT_END,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "SCART_LEFT_END",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_LEFT_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "SCART_LEFT_LOOP",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_LEFT_START,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "SCART_LEFT_START",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_RIGHT_END,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "SCART_RIGHT_END",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_RIGHT_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "SCART_RIGHT_LOOP",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_RIGHT_START,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "SCART_RIGHT_START",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCART_ROW,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "SCART_ROW",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::SCANNER_LOOP,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "SCANNER_LOOP",
        "",
        "",
        1.0f,
        1.0f
    );
}

void TCompPlayerAnimator::registerMsgs() {
    DECL_MSG(TCompPlayerAnimator, TMsgEntityCreated, onCreated);
    DECL_MSG(TCompPlayerAnimator, TMsgExecuteAnimation, playMsgAnimation);

}

void TCompPlayerAnimator::update(float dt) {

    std::list<EAnimation>::iterator it = animationCallbackRequests.begin();
    while (it != animationCallbackRequests.end())
    {
        if (!isPlayingAnimation((TCompAnimator::EAnimation) * it))
        {
            //Send animation finished message
            TMsgPlayerAnimationFinished msg;
            msg.animation = *it;
            CEntity* e = ((CEntity*)CHandle(this).getOwner());
            if (!e) {
              //Remove callback request
              animationCallbackRequests.erase(it++);
              continue;
            }
              
            e->sendMsg(msg);
            //Remove callback request
            animationCallbackRequests.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

void TCompPlayerAnimator::onCreated(const TMsgEntityCreated& msg) {

    ownHandle = CHandle(this).getOwner();
    CEntity* e = ownHandle;
    compSkeleton = e->get<TCompSkeleton>();
    assert(compSkeleton);
    initializeAnimations();
}

bool TCompPlayerAnimator::playAnimation(TCompPlayerAnimator::EAnimation animation, float speed, bool callbackWanted, float weight) {
    if (callbackWanted)
        animationCallbackRequests.push_back(animation);

    return playAnimationConverted((TCompAnimator::EAnimation)animation, speed, weight);
}

void TCompPlayerAnimator::playMsgAnimation(const TMsgExecuteAnimation& msg) {

    playAnimation(msg.animation, msg.speed);
}