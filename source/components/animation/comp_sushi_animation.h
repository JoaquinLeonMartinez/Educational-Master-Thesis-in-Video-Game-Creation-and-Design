#pragma once

#include "components/common/comp_base.h"
#include "skeleton/comp_skeleton.h"
#include "skeleton/comp_animator.h"
#include "entity/common_msgs.h"

class TCompSushiAnimator;

class TCompSushiAnimator : public TCompAnimator
{
public:
	enum EAnimation { 
		IDLE_LOOP = 0, 
		WALK_LOOP, 
		WAKEUP, 
		ATTACK1, 
		ATTACK2, 
		ATTACK3, 
		BLOCK_LOOP, 
		JUMP_START, 
		JUMP_LOOP, 
		JUMP_END, 
		JUMPCHARGE_START,
		JUMPCHARGE_LOOP,
		JUMPCHARGE_END,
		THROW_AIR,
		THROW_LAND,
        BLOCK_BREAK,
        BLOCK_HIT,
        DAMAGED,
        DEAD,
        GET_UP,
        BLOCK_BREAK_GET_UP,
		PRUEBA
	};
	struct TMsgExecuteAnimation {

		TCompSushiAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};

	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);
    bool isPlaying(TCompSushiAnimator::EAnimation animation);
	void initializeAnimations();
	bool playAnimation(TCompSushiAnimator::EAnimation animation, float speed = 1.0f);
};
