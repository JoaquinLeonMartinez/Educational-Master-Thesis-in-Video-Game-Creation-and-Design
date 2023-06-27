#include "mcv_platform.h"
#include "fsm/module_fsm.h"
#include "fsm/state.h"
#include "fsm/transition.h"

#include "fsm/customStates/idleAction.h"
#include "fsm/customStates/walkAction.h"
#include "fsm/customStates/jumpAction.h"

#include "fsm/customTransitions/checkVariable.h"
#include "fsm/customTransitions/checkTime.h"

namespace FSM
{
  CModuleFSM::CModuleFSM(const std::string& name)
    : IModule(name)
  {
  }

  bool CModuleFSM::start()
  {
    _stateFactories["idleAction"] = new StateFactory<CIdleAction>();
    _stateFactories["walkAction"] = new StateFactory<CWalkAction>();
    _stateFactories["jumpAction"] = new StateFactory<CJumpAction>();

    _transitionFactories["checkVariable"] = new TransitionFactory<CCheckVariable>();
    _transitionFactories["checkTime"] = new TransitionFactory<CCheckTime>();

    return true;
  }

  IState* CModuleFSM::createState(const std::string& type)
  {
    const auto& factory = _stateFactories.find(type);
    if(factory == _stateFactories.end())
    {
      return nullptr;
    }
    IState* st = factory->second->create();
    st->_type = factory->first;

    return st;
  }

  ITransition* CModuleFSM::createTransition(const std::string& type)
  {
    const auto& factory = _transitionFactories.find(type);
    if (factory == _transitionFactories.end())
    {
      return nullptr;
    }
    ITransition* tr = factory->second->create();
    tr->_type = factory->first;

    return tr;
  }

  void CModuleFSM::renderInMenu()
  {
    /*const auto printButton = [](const TButton& button, const char* label, bool centerValue = false)
    {
      float value = centerValue ? 0.5f + button.value * 0.5f : button.value;
      ImGui::ProgressBar(value, ImVec2(-1, 0), label);
    };

    if (ImGui::TreeNode("Input"))
    {
      if (ImGui::TreeNode("keyboard"))
      {
        for(int i = 0; i< Input::BT_KEYBOARD_COUNT; ++i)
        {
          printButton(_keyboard._buttons[i], getButtonName(INTERFACE_KEYBOARD, i).c_str());
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("mouse"))
      {
        ImGui::Text("Current position: %.0f %.0f", _mouse._currPosition.x, _mouse._currPosition.y);
        ImGui::Text("Previous position: %.0f %.0f", _mouse._prevPosition.x, _mouse._prevPosition.y);
        for (int i = 0; i< Input::BT_MOUSE_COUNT; ++i)
        {
          printButton(_mouse._buttons[i], getButtonName(INTERFACE_MOUSE, i).c_str());
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("pad"))
      {
        ImGui::Text("Connected: %s", _gamepad._connected ? "YES" : "no");
        for (int i = 0; i< Input::BT_GAMEPAD_COUNT; ++i)
        {
          bool isAnalog = i == Input::BT_LANALOG_X || i == Input::BT_LANALOG_Y || i == Input::BT_RANALOG_X || i == Input::BT_RANALOG_Y;
          printButton(_gamepad._buttons[i], getButtonName(INTERFACE_GAMEPAD, i).c_str(), isAnalog);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Mapping"))
      {
        for (auto& bt : _mapping._buttons)
        {
          printButton(bt.second.result, bt.first.c_str());
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }*/
  }
}
