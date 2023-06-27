#include "mcv_platform.h"
#include "comp_animator.h"
#include "components/common/comp_transform.h"


DECL_OBJ_MANAGER("animator", TCompAnimator);

void TCompAnimator::debugInMenu() {

}

void TCompAnimator::load(const json& j, TEntityParseContext& ctx) {

}



//inicializar animacion
bool TCompAnimator::initializeAnimation(EAnimation animation, EAnimationType animationType, EAnimationSize animationSize, std::string animationName, std::string secondAnimationName, std::string thirdAnimationName, float weight, float speed) {

	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	AnimationGroup auxAnimSet;
	auxAnimSet.animation = animation;
	auxAnimSet.animationType = animationType;
	auxAnimSet.animationSize = animationSize;
	auxAnimSet.animationName = animationName;
	auxAnimSet.animationId = compSkeleton->getAnimationIdByName(auxAnimSet.animationName);
	if (auxAnimSet.animationId == -1) {
		fatal("The first animation does not exist");
		return false;
	}
	auxAnimSet.secondAnimationName = secondAnimationName;
	auxAnimSet.secondAnimationId = compSkeleton->getAnimationIdByName(auxAnimSet.secondAnimationName);
	if (auxAnimSet.animationSize == EAnimationSize::DOUBLE && auxAnimSet.secondAnimationId == -1) {
		fatal("The second animation does not exist");
		return false;
	}
  auxAnimSet.thirdAnimationName = thirdAnimationName;
  auxAnimSet.thirdAnimationId = compSkeleton->getAnimationIdByName(auxAnimSet.thirdAnimationName);
  if (auxAnimSet.animationSize == EAnimationSize::TRIPLE && auxAnimSet.thirdAnimationId == -1) {
      fatal("The third animation does not exist");
      return false;
  }
	auxAnimSet.weight = weight;
	auxAnimSet.speed = speed;

	stringToAnimationsMap[animationName] = animation;
	animationsMap[animation] = auxAnimSet;
	return false;
}

void TCompAnimator::update(float dt)
{
}


bool TCompAnimator::playAnimationConverted(EAnimation animation, float speed, float weight) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		return false;
	}
	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	AnimationGroup animSet = animationsMap[animation];

	if (animSet.animationType == TCompAnimator::EAnimationType::CYCLIC && this->isPlayingAnimation(animSet.animation)) {
		return false;
	}

	int anim1id = animSet.animationId;
	int anim2id = animSet.secondAnimationId;
    float aux_weight = animSet.weight;
    if (weight != -1.0f) aux_weight = weight;
    float aux_speed = animSet.speed;
	if (speed != 1.0f) aux_speed = speed;

	switch (animSet.animationType)
	{
	case EAnimationType::CYCLIC:
		compSkeleton->changeCyclicAnimation(anim1id, aux_speed, anim2id, aux_weight);
		break;

	case EAnimationType::ACTION:
		compSkeleton->executeActionAnimation(anim1id, aux_speed);
		break;
	}

}

bool  TCompAnimator::isCyclic(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationGroup animSet = animationsMap[animation];
	return animSet.animationType == EAnimationType::CYCLIC;
}

bool  TCompAnimator::isComposed(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationGroup animSet = animationsMap[animation];
	return animSet.animationSize == EAnimationSize::DOUBLE;
}

TCompAnimator::EAnimation TCompAnimator::actualAnimation() {

	return actualCyclicAnimation.animation;
}

float  TCompAnimator::getAnimationDuration(EAnimation animation) {

	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationGroup animSet = animationsMap[animation];

	return compSkeleton->getAnimationDuration(animSet.animationId);
}

bool TCompAnimator::isPlayingAnimation(EAnimation animation) {
	CEntity* e = ownHandle;
  if (!e)
    return false;
	TCompSkeleton* compSkeleton = e->get<TCompSkeleton>();
	if (compSkeleton == NULL) {
		return false;
	}
	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationGroup animSet = animationsMap[animation];
	bool toReturn = false;
	if (animSet.animationType == TCompAnimator::EAnimationType::ACTION) {
		toReturn = compSkeleton->isExecutingActionAnimation(animSet.animationName);
	}
	if (animSet.animationType == TCompAnimator::EAnimationType::CYCLIC) {
		toReturn = compSkeleton->isExecutingCyclicAnimation(animSet.animationId);
	}
	return toReturn;
}

TCompAnimator::EAnimation TCompAnimator::getAnimationByName(std::string animation_name) {

	return stringToAnimationsMap[animation_name];
}