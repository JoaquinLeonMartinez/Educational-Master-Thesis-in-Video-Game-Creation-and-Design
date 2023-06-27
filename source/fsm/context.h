#pragma once

#include "fsm/variable.h"

class CFSM;
class IState;

class CFSMContext
{
public:
  void init(const CFSM* fsm);
  void update(float dt);
  
  TVariable* getVariable(const std::string& name);
  const TVariable* getVariable(const std::string& name) const;
  void setVariable(const TVariable& var);
  
  float getTimeInState() const { return _timeInState; }

  void debugInMenu();

private:
  void start();
  void stop();
  void changeToState(const IState* newState);

  const CFSM* _fsm = nullptr;
  const IState* _currentState = nullptr;
  std::map<std::string, TVariable> _variables;
  float _timeInState = 0.f;
};
