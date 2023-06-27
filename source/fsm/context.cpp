#include "mcv_platform.h"
#include "fsm/context.h"
#include "fsm/fsm.h"
#include "fsm/state.h"
#include "fsm/transition.h"
#include "fsm/variable.h"

void CFSMContext::init(const CFSM* fsm)
{
  stop();

  _fsm = fsm;
  for (auto& var : _fsm->getVariables())
  {
    setVariable(var);
  }

  start();
}

void CFSMContext::update(float dt)
{
  if (_currentState)
  {
    _timeInState += dt;
    _currentState->update(*this, dt);

    for (const auto& tr : _currentState->_transitions)
    {
      if (tr->check(*this))
      {
        changeToState(tr->_target);
        break;
      }
    }
  }
}

void CFSMContext::changeToState(const IState* newState)
{
  if (_currentState)
  {
    _currentState->onExit(*this);
  }

  _currentState = newState;
  _timeInState = 0.f;

  if (_currentState)
  {
    _currentState->onEnter(*this);
  }
}

void CFSMContext::start()
{
  _currentState = nullptr;
  if (_fsm)
  {
    changeToState(_fsm->getStartingState());
  }
}

void CFSMContext::stop()
{
  changeToState(nullptr);
}

TVariable* CFSMContext::getVariable(const std::string& name)
{
  const TVariable* var = const_cast<const CFSMContext*>(this)->getVariable(name);
  return const_cast<TVariable*>(var);
}

const TVariable* CFSMContext::getVariable(const std::string& name) const
{
  auto it = _variables.find(name);
  return it != _variables.end() ? &it->second : nullptr;
}

void CFSMContext::setVariable(const TVariable& var)
{
  _variables[var._name] = var;
}

void CFSMContext::debugInMenu()
{
  ImGui::Text("Current state: %s", _currentState ? _currentState->_name.c_str() : "...");
  ImGui::Text("Time in state: %.2f", _timeInState);
  ImGui::Separator();

  if (ImGui::TreeNode("Variables"))
  {
    for (auto& var : _variables)
    {
      ImGui::Text("%s [%s] : %s", var.first.c_str(), var.second.getType().c_str(), var.second.toString().c_str());
    }

    ImGui::TreePop();
  }
}
