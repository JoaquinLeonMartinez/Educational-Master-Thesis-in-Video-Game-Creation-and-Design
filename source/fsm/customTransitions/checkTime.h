#pragma once

#include "mcv_platform.h"
#include "fsm/transition.h"

class CCheckTime : public ITransition
{
public:
  void load(const json& jData) override;
  bool check(const CFSMContext& ctx) const override;

private:
  float _duration = 0.f;
};
