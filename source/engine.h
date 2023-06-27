#pragma once
#include "modules/module_manager.h"
#include "modules/module_entities.h"
#include "modules/module_physics.h"
#include "modules/module_multithreading.h"
#include "render/module_gpu_culling.h"
#include "modules/game/module_game_controller.h"
#include "modules/game/module_camera_shaker.h"
#include "modules/game/module_scripting.h"
#include "modules/game/module_audio.h"
#include "modules/module_navmesh.h"
#include "modules/module_scenes.h"
#include "modules/module_boot.h"

#define DECL_MODULE(__CLASS__, __MEMBER__, __GETTER__) \
  public: __CLASS__& __GETTER__() const { return *__MEMBER__; } \
  private: __CLASS__* __MEMBER__ = nullptr;

namespace Input
{
  class CModuleInput;
}
class CModuleRender;
class CModuleCameraMixer;
class CModuleLogic;
class CModuleCameraShaker;
class CModuleFluidDecalGenerator;
class CModuleNavmesh;
class CModuleSceneManager;
class CModuleBoot;

namespace UI
{
  class CModuleUI;
}
namespace particles
{
  class CModuleParticles;
}

class CEngine
{
  CModuleManager   _modules;

public:
  CEngine();
  ~CEngine();

  static CEngine& get();

  void start();
  void stop();
  void update(float dt);
  void renderDebug();
  void registerResourceTypes();

  CModuleManager& getModules();

  DECL_MODULE(Input::CModuleInput, _input, getInput);
  DECL_MODULE(CModuleRender, _render, getRender);
  DECL_MODULE(CModuleEntities, _entities, getEntities);
  DECL_MODULE(CModulePhysics, _physics, getPhysics);
  DECL_MODULE(CModuleCameraMixer, _cameraMixer, getCameraMixer);
  DECL_MODULE(MultithreadingModule, _multithreading, getMultithreading);
  DECL_MODULE(CModuleGameController, _gameController, getGameController);
  DECL_MODULE(CModuleNavmesh, _navmesh, getNavmesh);
  DECL_MODULE(CModuleAudio, _audioManager, getAudioManager);
  DECL_MODULE(CModuleCameraShaker, _cameraShaker, getCameraShaker);
  DECL_MODULE(CModuleScripting, _scripting, getScripting);
  DECL_MODULE(CModuleFluidDecalGenerator, _fluidDecals, getFluidDecalGenerator);
  
  DECL_MODULE(CModuleGPUCulling, _gpu_culling, getGPUCulling);
  DECL_MODULE(UI::CModuleUI, _ui, getUI);
  DECL_MODULE(particles::CModuleParticles, _particles, getParticles);
  DECL_MODULE(CModuleSceneManager, _sceneManager, getSceneManager);
  DECL_MODULE(CModuleBoot, _boot, getBoot);



};

#define Engine CEngine::get()
#define EngineInput CEngine::get().getInput()
#define EnginePhysics CEngine::get().getPhysics()
#define EngineEntities CEngine::get().getEntities()
#define GameController CEngine::get().getGameController()
#define EngineAudio CEngine::get().getAudioManager()
#define CameraShaker CEngine::get().getCameraShaker()
#define Scripting CEngine::get().getScripting()
#define FluidDecalGenerator CEngine::get().getFluidDecalGenerator()
#define EngineNavmesh CEngine::get().getNavmesh()
#define SceneManager CEngine::get().getSceneManager()
#define Boot CEngine::get().getBoot()
