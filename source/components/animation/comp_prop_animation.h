#pragma once

#include "components/common/comp_base.h"
#include "skeleton/comp_skeleton.h"
#include "skeleton/comp_animator.h"
#include "entity/common_msgs.h"

class TCompPropAnimator;

class TCompPropAnimator : public TCompAnimator
{
public:
	enum EAnimation { 
      OVEN_IDLE = 0,
      OVEN_OPEN,
			OVEN_BROKEN,
			OVEN_MEDIO_ABIERTO
	};
	struct TMsgExecuteAnimation {

      TCompPropAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};

	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);
  bool isPlaying(TCompPropAnimator::EAnimation animation);
	void initializeAnimations();
	bool playAnimation(TCompPropAnimator::EAnimation animation, float speed = 1.0f);
};
