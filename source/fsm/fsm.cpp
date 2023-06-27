#include "mcv_platform.h"
#include "fsm/fsm.h"
#include "fsm/state.h"
#include "fsm/transition.h"
#include "fsm/variable.h"
#include "engine.h"
#include "fsm/module_fsm.h"

class CFSMResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "fsm"; }
  const char* getName() const override {
    return "FSM's";
  }
  IResource* create(const std::string& name) const override {
    CFSM* newFsm = new CFSM();
    bool is_ok = newFsm->load(name);
    assert(is_ok);
    newFsm->setNameAndType(name, this);
    return newFsm;
  }
};

template<>
const CResourceType* getResourceTypeFor<CFSM>() {
  static CFSMResourceType resource_type;
  return &resource_type;
}

bool CFSM::load(const std::string& file)
{
  FSM::CModuleFSM& factories = Engine.getFSM();

  json jData = loadJson(file);

  const json& jStates = jData["states"];
  const json& jTransitions = jData["transitions"];
  const json& jVariables = jData["variables"];

  for (const auto& jState : jStates)
  {
    const std::string stName = jState["name"];
    const std::string stType = jState["type"];

    IState* newState = factories.createState(stType);
    newState->_name = stName;
    newState->load(jState);

    _states.push_back(newState);
  }

  for (const auto& jTransition : jTransitions)
  {
    const std::string trSource = jTransition["source"];
    const std::string trTarget = jTransition["target"];
    const std::string trType = jTransition["type"];

    IState* stSource = getState(trSource);

    ITransition* newTransition = factories.createTransition(trType);

    newTransition->_source = stSource;
    newTransition->_target = getState(trTarget);
    newTransition->load(jTransition);

    _transitions.push_back(newTransition);

    if (stSource)
    {
      stSource->_transitions.push_back(newTransition);
    }
  }

  for (const auto& jVariable : jVariables)
  {
    TVariable newVariable = TVariable::parseVariable(jVariable);

    _variables.push_back(std::move(newVariable));
  }

  const std::string startingState = jData["start"];
  _startState = getState(startingState);

  return true;
}

IState* CFSM::getState(const std::string& name) const
{
  auto it = std::find_if(_states.begin(), _states.end(), [&name](const IState* state)
  {
    return state->_name == name;
  });
  return it != _states.end() ? *it : nullptr;
}

void CFSM::renderInMenu()
{
  ImGui::Text("Start state: %s", _startState ? _startState->_name.c_str() : "...");
  if (ImGui::TreeNode("States"))
  {
    for (auto& st : _states)
    {
      if (ImGui::TreeNode("%s [%s]", st->_name.c_str(), st->_type.data()))
      {
        for (auto& tr : st->_transitions)
        {
          ImGui::Text("-> %s [%s]", tr->_target ? tr->_target->_name.c_str() : "...", tr->_type.data());
        }

        ImGui::TreePop();
      }
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Variables"))
  {
    for (auto& var : _variables)
    {
      ImGui::Text("%s [%s] : %s", var._name.c_str(), var.getType().c_str(), var.toString().c_str());
    }

    ImGui::TreePop();
  }
}
