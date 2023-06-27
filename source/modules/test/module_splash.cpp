#include "mcv_platform.h"
#include "module_splash.h"
#include "engine.h"

CModuleSplash::CModuleSplash(const std::string& name)
  : IModule(name)
{}

bool CModuleSplash::start()
{
  _timer = 3.f;
  return true;
}

void CModuleSplash::stop()
{
}

void CModuleSplash::update(float dt)
{
  _timer -= dt;
  if (_timer <= 0.f)
  {
    CEngine::get().getModules().changeToGamestate("gs_main_menu");
  }
}
