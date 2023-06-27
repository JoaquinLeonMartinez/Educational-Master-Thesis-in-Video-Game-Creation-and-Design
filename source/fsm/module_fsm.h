#pragma once

#include "modules/module.h"

class IState;
class ITransition;

namespace FSM
{
  // STATES
  class IStateFactory
  {
  public:
    virtual IState* create() = 0;
  };

  template<class StateType>
  class StateFactory : public IStateFactory
  {
  public:
    IState* create() override
    {
      return new StateType;
    }
  };

  // TRANSITIONS
  class ITransitionFactory
  {
  public:
    virtual ITransition* create() = 0;
  };

  template<class TransitionType>
  class TransitionFactory : public ITransitionFactory
  {
  public:
    ITransition* create() override
    {
      return new TransitionType;
    }
  };

  class CModuleFSM : public IModule
  {
  public:
    CModuleFSM(const std::string& name);

    bool start() override;
    void renderInMenu() override;

    IState* createState(const std::string& type);
    ITransition* createTransition(const std::string& type);

  private:
    std::map<std::string, IStateFactory*> _stateFactories;
    std::map<std::string, ITransitionFactory*> _transitionFactories;
  };
}
