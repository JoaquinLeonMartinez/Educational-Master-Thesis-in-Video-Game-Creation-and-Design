#pragma once

#include "mcv_platform.h"

class CFSMContext;
class ITransition;

class IState
{
public:
  std::string _name;
  std::string_view _type;
  std::vector<const ITransition*> _transitions;

  virtual void load(const json& jData) {}

  virtual void update(CFSMContext& ctx, float dt) const {}
  virtual void onEnter(CFSMContext& ctx) const {}
  virtual void onExit(CFSMContext& ctx) const {}

private:
};
