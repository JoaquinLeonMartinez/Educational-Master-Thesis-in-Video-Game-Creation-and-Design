#pragma once

#include "mcv_platform.h"
#include "fsm/transition.h"
#include "fsm/variable.h"

class CCheckVariable : public ITransition
{
public:
  void load(const json& jData) override;
  bool check(const CFSMContext& ctx) const override;

private:
  TVariable _variable;
  EVariableOperator _operator = EVariableOperator::Equal;
};
