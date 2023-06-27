#pragma once

#include "mcv_platform.h"
#include "fsm/state.h"

class CJumpAction : public IState
{
public:
  void update(CFSMContext& ctx, float dt) const override;
  void onEnter(CFSMContext& ctx) const override;
  void onExit(CFSMContext& ctx) const override;
};
