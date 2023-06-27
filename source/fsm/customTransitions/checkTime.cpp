#include "mcv_platform.h"
#include "fsm/customTransitions/checkTime.h"
#include "fsm/context.h"

void CCheckTime::load(const json& jData)
{
  _duration = jData.value<float>("duration", 0.f);
}

bool CCheckTime::check(const CFSMContext& ctx) const
{
  const float timeInState = ctx.getTimeInState();
  return timeInState >= _duration;
}
