#include "mcv_platform.h"
#include "module_manager.h"
#include "utils/json.hpp"
#include <fstream>

void CModuleManager::start()
{
  loadConfig();
  loadGamestates();

  startModules(_system_modules);

  if (!_startGamestate.empty())
  {
    changeToGamestate(_startGamestate);
  }
}

void CModuleManager::stop()
{
  if (_currentGamestate)
  {
    stopModules(*_currentGamestate);
  }
  stopModules(_system_modules); // better in reverse order
}

void CModuleManager::update(float dt)
{
  PROFILE_FUNCTION("CModuleManager::update");
  updateGamestate();

  for (auto module : _update_modules) {
    if (!module->isActive()) continue;
    PROFILE_FUNCTION(module->getName().c_str());
	module->update(dt);
  }
}

void CModuleManager::renderDebug()
{
  CGpuScope gpu_trace("Modules::renderDebug");
  for (auto module : _render_debug_modules) {
    if (!module->isActive()) continue;
    CGpuScope gpu_trace(module->getName().c_str());
    module->renderDebug();
  }
}

void CModuleManager::registerGameModule( IModule* module ) 
{
  _all_modules.push_back(module);
}

void CModuleManager::registerSystemModule(IModule* module) 
{
  _all_modules.push_back(module);
  _system_modules.push_back(module);
}

void CModuleManager::changeToGamestate(const std::string& name)
{
  CGamestate* gs = getGamestate(name);
  if (!gs)
  {
    return;
  }

  _requestedGamestate = gs;
}

IModule* CModuleManager::getModule(const std::string& name)
{
  for (auto module : _all_modules) {
    if (module->getName() == name)
    {
      return module;
    }
  }
  return nullptr;
}

void CModuleManager::startModules( VModules& modules )
{
  PROFILE_FUNCTION("Modules::Start");
  for (auto module : modules) {
    if (module->isActive()) continue;
    TFileContext fc(module->getName());
    PROFILE_FUNCTION(module->getName().c_str());
    module->start();
    module->setActive(true);
  }
}

void CModuleManager::stopModules(VModules& modules)
{
	dbg("-------\n");
  for (auto module : modules) {
    if (!module->isActive()) continue;
	dbg("stop %s modules\n", module->getName().c_str());
    module->stop();
    module->setActive(false);
	
  }
}

CGamestate* CModuleManager::getGamestate(const std::string& name)
{
  for (auto& gs : _gamestates)
  {
    if (gs._name == name)
    {
      return &gs;
    }
  }
  return nullptr;
}

void CModuleManager::updateGamestate()
{
  if (!_requestedGamestate)
  {
    return;
  }

  //TOCAR AQUI DANI
  //PARAR SOLO LOS MODULOS QUE NO ESTÉN EN EL NUEVO GAMESTATE
  if (_currentGamestate)
  {
    CGamestate modulesToStop;

    for (auto currentModule : *_currentGamestate) {
      bool found = false;
      for (auto requestedModule : *_requestedGamestate) {
        if (requestedModule->getName() == currentModule->getName()) {
          found = true;
          break;
        }
      }
      if (!found) {
        modulesToStop.push_back(currentModule);
      }
    }

    if (!modulesToStop.empty()) {
      dbg("stop %i modules\n",modulesToStop.size());
      stopModules(modulesToStop);
    }
  }

  // INICIAR LOS MODULOS QUE NO ESTABAN EN EL ANTERIOR GAMESTATE
  CGamestate modulesToStart;
  for (auto requestedModule : *_requestedGamestate) {
    bool found = false;
    if(_currentGamestate){
      for (auto currentModule : *_currentGamestate) {
        if (requestedModule->getName() == currentModule->getName()) {
          found = true;
          break;
        }
      }
    }
    if (!found) {
      modulesToStart.push_back(requestedModule);
    }
  }

  if (!modulesToStart.empty()) {
    dbg("start %i modules\n", modulesToStart.size());
    startModules(modulesToStart);
  }
  _currentGamestate = _requestedGamestate;
  _requestedGamestate = nullptr;
}

void CModuleManager::loadConfig()
{
  json jsonData = loadJson( "data/modules.json");

  _update_modules.clear();
  _render_debug_modules.clear();

  for (auto& moduleName : jsonData["update"])
  {
    IModule* module = getModule(moduleName);
    if(module)
    {
      _update_modules.push_back(module);
    }
  }

  for (auto& moduleName : jsonData["render_debug"])
  {
    IModule* module = getModule(moduleName);
    if (module)
    {
      _render_debug_modules.push_back(module);
    }
  }
}

void CModuleManager::loadGamestates()
{
  json jsonData = loadJson("data/gamestates.json");
  json& jsonGamestates = jsonData["gamestates"];

  for (auto& jsonGs : jsonGamestates.items())
  {
    CGamestate gs;

    gs._name = jsonGs.key();
    for (auto& jsonModule : jsonGs.value())
    {
      IModule* module = getModule(jsonModule);
      gs.push_back(module);
    }
    _gamestates.push_back(gs);
  }

  _startGamestate = jsonData["start"].get<std::string>();
}

void CModuleManager::renderInMenu()
{
  auto printModule = [](const IModule* module)
  {
    ImGui::Text("%s %s", module->getName().c_str(), module->isActive() ? "[active]" : "");
  };
  auto printModules = [&printModule](const char* groupName, const VModules& modules)
  {
    if (ImGui::TreeNode(groupName))
    {
      for (auto& module : modules)
      {
        printModule(module);
      }
      ImGui::TreePop();
    }
  };

  if (ImGui::TreeNode("Modules"))
  {
    if (ImGui::TreeNode("Modules"))
    {
      printModules("All", _all_modules);
      printModules("System", _system_modules);
      printModules("Update", _update_modules);
      printModules("Render", _render_debug_modules);
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Gamestates"))
    {
      ImGui::Text("Start: %s", _startGamestate.c_str());

      if (ImGui::BeginCombo("Current", _currentGamestate ? _currentGamestate->_name.c_str() : "..."))
      {
        for (auto& gs : _gamestates)
        {
          if (ImGui::Selectable(gs._name.c_str(), &gs == _currentGamestate))
          {
            changeToGamestate(gs._name);
          }
        }
        ImGui::EndCombo();
      }

      for (auto& gs : _gamestates)
      {
        printModules(gs._name.c_str(), gs);
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }

  for (auto module : _all_modules) {
    if (!module->isActive()) continue;
    module->renderInMenu();
  }
}

void CModuleManager::setTimeScale(float timeScale) {
	assert(timeScale >= 0.0);
	_timeScale = timeScale;
}
float CModuleManager::getTimeScale() {
	return _timeScale;
}

//void CModuleManager::setPlayerTimeScale(float timeScale) {
//	assert(timeScale >= 0.0);
//	_playerTimeScale = timeScale;
//}
//float CModuleManager::getPlayerTimeScale() {
//	return _playerTimeScale;
//}