#pragma once
#include "module.h"
#include "gamestate.h"

class CModuleManager
{
  VModules _all_modules;
  VModules _system_modules;
  VModules _update_modules;
  VModules _render_debug_modules;
  VGamestates _gamestates;
  CGamestate* _currentGamestate = nullptr;
  CGamestate* _requestedGamestate = nullptr;
  std::string _startGamestate;

  void startModules(VModules& modules);
  void stopModules(VModules& modules);

  void updateGamestate();

  float _timeScale = 1.0f;
  //float _playerTimeScale = 1.0f;

public:
  void start();
  void stop();

  void registerGameModule(IModule* module);
  void registerSystemModule(IModule* module);

  void changeToGamestate(const std::string& name);

  void update(float dt);
  void renderDebug();
  void renderInMenu();

  IModule* getModule(const std::string& name);
  CGamestate* getGamestate(const std::string& name);

  void loadConfig();
  void loadGamestates();

  void setTimeScale(float timeScale);
  float getTimeScale();
  //void setPlayerTimeScale(float timeScale);
  //float getPlayerTimeScale();
};
