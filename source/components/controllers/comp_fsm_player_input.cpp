#include "mcv_platform.h"
#include "components/controllers/comp_fsm_player_input.h"
#include "components/common/comp_fsm.h"
#include "engine.h"
#include "input/input.h"

DECL_OBJ_MANAGER("fsm_player_input", TCompFSMPlayerInput);

void TCompFSMPlayerInput::update(float scaled_dt)
{
  auto processBt = [this](const std::string& btName, const std::string& varName, bool isBool)
  {
    const Input::TButton& bt = EngineInput[btName];
    if (bt.justChanged())
    {
      TVariable var;
      var._name = varName;
      if (isBool)
      {
        var._value = bt.isPressed();
      }
      else
      {
        var._value = bt.value;
      }

      TCompFSM* cFsm = get<TCompFSM>();
      if (cFsm)
      {
        cFsm->setVariable(var);
      }
    }
  };

  processBt("walk", "speed", false);
  processBt("jump", "jump", true);
}
