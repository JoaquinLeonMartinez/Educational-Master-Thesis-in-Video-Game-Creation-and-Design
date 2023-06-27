#include "mcv_platform.h"
#include "comp_prop_animation.h"
#include "components/common/comp_transform.h"


DECL_OBJ_MANAGER("prop_animator", TCompPropAnimator);

void TCompPropAnimator::debugInMenu() {

    static float delta_movement = 0.0f;
    static float speed = 1.0f;
    ImGui::DragFloat("Speed", &speed, 0.01f, 0, 3.f);
    if (ImGui::SmallButton("Idle")) {
        playAnimation(EAnimation::OVEN_IDLE, speed);
    }


    ImGui::DragFloat("Delta Movement", &delta_movement, 0.01f, 0, 1.f);
    TCompSkeleton * compSkeleton = get<TCompSkeleton>();
    compSkeleton->setCyclicAnimationWeight(delta_movement);
}

void TCompPropAnimator::initializeAnimations() {

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::OVEN_IDLE,
        EAnimationType::CYCLIC,
        EAnimationSize::SINGLE,
        "OVEN_IDLE",
        "",
        "",
        1.0f,
        1.0f
    );

    initializeAnimation(
        (TCompAnimator::EAnimation)EAnimation::OVEN_OPEN,
        EAnimationType::ACTION,
        EAnimationSize::SINGLE,
        "OVEN_OPEN",
        "",
        "",
        1.0f,
        1.0f
    );
		
		initializeAnimation(
			(TCompAnimator::EAnimation)EAnimation::OVEN_BROKEN,
			EAnimationType::CYCLIC,
			EAnimationSize::SINGLE,
			"OVEN_BROKEN",
			"",
			"",
			1.0f,
			1.0f
		);

		initializeAnimation(
			(TCompAnimator::EAnimation)EAnimation::OVEN_MEDIO_ABIERTO,
			EAnimationType::CYCLIC,
			EAnimationSize::SINGLE,
			"OVEN_MEDIO_ABIERTO",
			"",
			"",
			1.0f,
			1.0f
		);
}

void TCompPropAnimator::registerMsgs() {
    DECL_MSG(TCompPropAnimator, TMsgEntityCreated, onCreated);
    DECL_MSG(TCompPropAnimator, TMsgExecuteAnimation, playMsgAnimation);

}


void TCompPropAnimator::onCreated(const TMsgEntityCreated& msg) {

    ownHandle = CHandle(this).getOwner();
    CEntity *e = ownHandle;
    TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
    assert(compSkeleton);
    initializeAnimations();
}

bool TCompPropAnimator::playAnimation(TCompPropAnimator::EAnimation animation, float speed) {

    return playAnimationConverted((TCompAnimator::EAnimation)animation, speed);
}

bool TCompPropAnimator::isPlaying(TCompPropAnimator::EAnimation animation) {

    return isPlayingAnimation((TCompAnimator::EAnimation)animation);
}

void TCompPropAnimator::playMsgAnimation(const TMsgExecuteAnimation& msg) {

    playAnimation(msg.animation, msg.speed);
}