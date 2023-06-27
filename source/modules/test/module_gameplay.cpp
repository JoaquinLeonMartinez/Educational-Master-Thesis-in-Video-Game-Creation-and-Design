#include "mcv_platform.h"
#include "module_gameplay.h"
#include "engine.h"
#include "input/input.h"

CModuleGameplay::CModuleGameplay(const std::string& name)
  : IModule(name)
{}

bool CModuleGameplay::start()
{
  return true;
}

void CModuleGameplay::stop()
{
}

void CModuleGameplay::update(float dt)
{
  if (EngineInput["back"].justPressed())
  {
    CEngine::get().getModules().changeToGamestate("gs_main_menu");
  }

    /*Input::CModuleInput& input = CEngine::get().getInput();
    Input::TRumbleData rumble;
    rumble.leftVibration = input[Input::BT_LTRIGGER].value;
    rumble.rightVibration = input[Input::BT_RTRIGGER].value;
    input.feedback(rumble);*/
}
