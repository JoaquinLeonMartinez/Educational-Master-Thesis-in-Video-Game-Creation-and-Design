#pragma once

#include "components/common/comp_base.h"
#include "skeleton/comp_skeleton.h"
#include "skeleton/comp_animator.h"
#include "entity/common_msgs.h"

class TCompPlayerAnimator;

class TCompPlayerAnimator : public TCompAnimator
{
public:
	enum EAnimation { 
        IDLE_MELEE = 0, 
        IDLE_FIRE, 
        IDLE_COMBAT, 
        MELEE1_FULL, 
        MELEE1_PARTIAL, 
        MELEE2_FULL, 
        MELEE2_PARTIAL, 
        AIM_THROW, 
        DAMAGED, 
        DASH, 
        DOUBLE_JUMP, 
        DRINK, 
        JUMP_END, 
        JUMP_START, 
        ON_AIR, 
        RUN, 
        SCAN, 
        THROW, 
        WALK,
        DEAD,
        DIE,
        FIRE_END,
        FIRE_END_PARTIAL,
        FIRE_LOOP,
        FIRE_LOOP_PARTIAL,
        FIRE_START,
        FIRE_START_PARTIAL,
        CHARGED_MELEE_ATTACK,
        CHARGED_MELEE_LOOP,
        CHARGED_MELEE_POSE,
        SCART_IDLE,
        SCART_LEFT_END,
        SCART_LEFT_LOOP,
        SCART_LEFT_START,
        SCART_RIGHT_END,
        SCART_RIGHT_LOOP,
        SCART_RIGHT_START,
        SCART_ROW,
        SCANNER_LOOP
    };
	struct TMsgExecuteAnimation {

		TCompPlayerAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};

	struct TMsgPlayerAnimationFinished {

		TCompPlayerAnimator::EAnimation animation;
		DECL_MSG_ID();
	};

	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);
	void update(float dt);
	
	void initializeAnimations();
	bool playAnimation(TCompPlayerAnimator::EAnimation animation, float speed = 1.0f, bool callbackWanted = false, float weight = -1.0f);

	TCompSkeleton * compSkeleton;
	//A vector where we include those animations whose finish we want to be notified of 
	//We'll traverse the vector on each frame, comparing it with the mixer, to see if the animation
	//has left it. Once it leaves, we remove it from the vector and notify our entity
	std::list< EAnimation> animationCallbackRequests;
};
