#pragma once

#include "mcv_platform.h"
#include "components/common/comp_base.h"
#include "skeleton/comp_skeleton.h"

class TCompAnimator;

class TCompAnimator : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:

	enum EAnimation { DEFAULT = 0 };
	enum EAnimationType { ACTION = 0, CYCLIC };
	enum EAnimationSize { SINGLE = 0, DOUBLE, TRIPLE };

	struct AnimationGroup {

		EAnimation animation = EAnimation::DEFAULT;
		EAnimationType animationType = EAnimationType::ACTION;
		EAnimationSize animationSize = EAnimationSize::SINGLE;
		std::string animationName = "";
    std::string secondAnimationName = "";
    std::string thirdAnimationName = "";
		int animationId = -1;
    int secondAnimationId = -1;
    int thirdAnimationId = -1;
		float weight = 1.0f;
		float speed = 1.0f;
	};
	
	CHandle ownHandle;
	AnimationGroup actualCyclicAnimation;
	std::map<EAnimation, AnimationGroup> animationsMap;
	std::map<std::string, EAnimation> stringToAnimationsMap;

	float _time = 0.f;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	//void initializeAnimations();

	bool initializeAnimation(EAnimation animation, EAnimationType animationType, EAnimationSize animationSize, std::string animationName, std::string secondAnimationName = "", std::string thirdAnimationName = "", float weight = 1.0f, float speed = 1.0f);
	bool playAnimationConverted(EAnimation animation, float speed = 1.0f, float weight = -1.0f);
	bool isCyclic(EAnimation animation);
	bool isComposed(EAnimation animation);
	EAnimation actualAnimation();
	bool isPlayingAnimation(EAnimation animation);

	EAnimation getAnimationByName(std::string animation_name);
	float getAnimationDuration(EAnimation animation);

};