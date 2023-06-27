#include "mcv_platform.h"
#include "comp_golem_animation.h"
#include "components/common/comp_transform.h"


DECL_OBJ_MANAGER("golem_animator", TCompGolemAnimator);

void TCompGolemAnimator::debugInMenu() {

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

void TCompGolemAnimator::initializeAnimations() {

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
        (TCompAnimator::EAnimation)EAnimation::POUND,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "pound",
        "",
        "",
        0.1f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::POUND_ATTACK,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "pound_attack",
        "",
        "",
        1.0f,
        1.0f
    );
		
    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::FRUSTRATION,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "frustration",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::LIBERATION,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "liberation",
        "",
        "",
        1.0f,
        1.0f
    );
		
    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::THROW,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "throw",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::DAMAGE,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "damage",
        "",
        "",
        1.0f,
        1.0f
    );

 

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::RECOVER,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "recover",
        "",
        "",
        1.0f,
        1.0f
    );
}

void TCompGolemAnimator::registerMsgs() {
    DECL_MSG(TCompGolemAnimator, TMsgEntityCreated, onCreated);
    DECL_MSG(TCompGolemAnimator, TMsgExecuteAnimation, playMsgAnimation);

}


void TCompGolemAnimator::onCreated(const TMsgEntityCreated& msg) {
	

		ownHandle = CHandle(this).getOwner();
		CEntity *e = ownHandle;
		TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
		assert(compSkeleton);
		initializeAnimations();
}

bool TCompGolemAnimator::playAnimation(TCompGolemAnimator::EAnimation animation, float speed) {

    return playAnimationConverted((TCompAnimator::EAnimation)animation, speed);
}

bool TCompGolemAnimator::isPlaying(TCompGolemAnimator::EAnimation animation) {

    return isPlayingAnimation((TCompAnimator::EAnimation)animation);
}

void TCompGolemAnimator::playMsgAnimation(const TMsgExecuteAnimation& msg) {

    playAnimation(msg.animation, msg.speed);
}