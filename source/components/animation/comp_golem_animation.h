#pragma once

#include "components/common/comp_base.h"
#include "skeleton/comp_skeleton.h"
#include "skeleton/comp_animator.h"
#include "entity/common_msgs.h"

class TCompGolemAnimator;

class TCompGolemAnimator : public TCompAnimator
{
public:
	enum EAnimation { 
		IDLE_LOOP = 0, 
		POUND,
		POUND_ATTACK, 
		FRUSTRATION, 
		LIBERATION, 
		THROW, 
		DAMAGE, 
		RECOVER
		/*

Idle 0 - 60
Pound 65 - 95
Pound + Atasco 100 - 158
frustracion 158 - 178
Liberacion 178 - 235
Lanzar 235 - 277
damage/muerte 291 - 311
recover 311 - 329
	*/
	};
	struct TMsgExecuteAnimation {

		TCompGolemAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};

	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);
  bool isPlaying(TCompGolemAnimator::EAnimation animation);
	void initializeAnimations();
	bool playAnimation(TCompGolemAnimator::EAnimation animation, float speed = 1.0f);
};
