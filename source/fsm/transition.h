#pragma once

#include "mcv_platform.h"

class CFSMContext;
class IState;

class ITransition
{
public:
  virtual void load(const json& jData) {}
  virtual bool check(const CFSMContext& ctx) const { return false; }

  const IState* _source = nullptr;
  const IState* _target = nullptr;
  std::string_view _type;

private:

};
