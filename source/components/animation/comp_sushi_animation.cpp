#include "mcv_platform.h"
#include "comp_sushi_animation.h"
#include "components/common/comp_transform.h"


DECL_OBJ_MANAGER("sushi_animator", TCompSushiAnimator);

void TCompSushiAnimator::debugInMenu() {

    static float delta_movement = 0.0f;
    static float speed = 1.0f;
    ImGui::DragFloat("Speed", &speed, 0.01f, 0, 3.f);
    if (ImGui::SmallButton("Idle")) {
        playAnimation(EAnimation::IDLE_LOOP, speed);
    }


    ImGui::DragFloat("Delta Movement", &delta_movement, 0.01f, 0, 1.f);
    TCompSkeleton * compSkeleton = get<TCompSkeleton>();
    compSkeleton->setCyclicAnimationWeight(delta_movement);
}

void TCompSushiAnimator::initializeAnimations() {

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::IDLE_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "idle_loop",
        "",
        "",
        0.1f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::WALK_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "walk_loop",
        "",
        "",
        0.1f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::ATTACK1,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "attack1",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::ATTACK2,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "attack2",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::ATTACK3,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "attack3",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::WAKEUP,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "wakeup",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::BLOCK_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "block_loop",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMP_START,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "jump_start",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMP_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "jump_loop",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMP_END,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "jump_end",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMPCHARGE_START,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "jumpCharge_start",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMPCHARGE_LOOP,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "jumpCharge_loop",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::JUMPCHARGE_END,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "jumpCharge_end",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::THROW_AIR,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "throw_air",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::THROW_LAND,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "throw_land2",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::BLOCK_BREAK,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "BLOCK_BREAK",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::BLOCK_HIT,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "BLOCK_HIT",
        "",
        "",
        2.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DAMAGED,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "DAMAGED",
        "",
        "",
        2.0f,
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
        (TCompAnimator::EAnimation)EAnimation::GET_UP,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "GET_UP",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::BLOCK_BREAK_GET_UP,
        EAnimationType::ACTION,
        EAnimationSize::DOUBLE,
        "BLOCK_BREAK",
        "GET_UP",
        "",
        1.0f,
        1.0f
    );
}

void TCompSushiAnimator::registerMsgs() {
    DECL_MSG(TCompSushiAnimator, TMsgEntityCreated, onCreated);
    DECL_MSG(TCompSushiAnimator, TMsgExecuteAnimation, playMsgAnimation);

}


void TCompSushiAnimator::onCreated(const TMsgEntityCreated& msg) {

	
	
	  ownHandle = CHandle(this).getOwner();
    CEntity *e = ownHandle;
    TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
    assert(compSkeleton);
    initializeAnimations();
}

bool TCompSushiAnimator::playAnimation(TCompSushiAnimator::EAnimation animation, float speed) {

    return playAnimationConverted((TCompAnimator::EAnimation)animation, speed);
}

bool TCompSushiAnimator::isPlaying(TCompSushiAnimator::EAnimation animation) {

    return isPlayingAnimation((TCompAnimator::EAnimation)animation);
}

void TCompSushiAnimator::playMsgAnimation(const TMsgExecuteAnimation& msg) {

    playAnimation(msg.animation, msg.speed);
}