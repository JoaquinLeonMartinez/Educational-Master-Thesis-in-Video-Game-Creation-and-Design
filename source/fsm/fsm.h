#pragma once

#include "resources/resource.h"
#include "fsm/variable.h"

class IState;
class ITransition;

class CFSM : public IResource
{
public:
  bool load(const std::string& file);
  IState* getState(const std::string& name) const;
  const IState* getStartingState() const { return _startState; }
  const std::vector<TVariable>& getVariables() const { return _variables; }

  void renderInMenu() override;

private:
  std::vector<IState*> _states;
  std::vector<ITransition*> _transitions;
  std::vector<TVariable> _variables;
  const IState* _startState = nullptr;
};
