#include "mcv_platform.h"
#include "engine.h"

#include "resources/resource.h"
#include "render/module_render.h"
#include "render/textures/texture.h"
#include "render/textures/material.h"
#include "render/meshes/mesh.h"
#include "render/meshes/collision_mesh.h"
#include "render/shaders/technique.h"
#include "render/compute/compute_shader.h"
#include "skeleton/game_core_skeleton.h"
#include "geometry/curve.h"
#include "utils/json_resource.h"
#include "windows/app.h"
#include "render/module_render.h"
#include "render/module_gpu_culling.h"
#include "input/module_input.h"
#include "modules/module_camera_mixer.h"
#include "ui/module_ui.h"
#include "particles/module_particles.h"
#include "particles/particle_emitter.h"

#include "modules/test/module_splash.h"
#include "modules/test/module_main_menu.h"
#include "modules/test/module_gameplay.h"
#include "modules/test/module_sample_objs.h"
#include "modules/test/module_test_cameras.h"
#include "modules/game/module_fluid_decal_generator.h"
#include "modules/game/module_game_paused.h"
#include "modules/game/module_game_ui.h"
#include "modules/game/module_game_manager.h"
#include "modules/game/module_game_loading.h"
#include "modules/game/module_game_over.h"
#include "modules/game/module_win_game.h"
#include "modules/game/module_game_intro.h"
#include "modules/module_boot.h"

#include "input/devices/device_keyboard.h"
#include "input/devices/device_mouse.h"
#include "input/devices/device_pad_xbox.h"
#include "utils/directory_watcher.h"

#include "modules/game/module_audio.h"

CDirectoyWatcher dir_watcher_data;

CEngine::CEngine()
{
}

CEngine::~CEngine()
{
#pragma message("TODO: delete modules!!!")
}

CEngine& CEngine::get()
{
  static CEngine engine;
  return engine;
}

void CEngine::registerResourceTypes() {
  PROFILE_FUNCTION("registerResourceTypes");
  Resources.registerResourceType(getResourceTypeFor<CMesh>());
  Resources.registerResourceType(getResourceTypeFor<CJson>());
  Resources.registerResourceType(getResourceTypeFor<CTexture>());
  Resources.registerResourceType(getResourceTypeFor<CTechnique>());
  Resources.registerResourceType(getResourceTypeFor<CMaterial>());
  Resources.registerResourceType(getResourceTypeFor<CCollisionMesh>());
  Resources.registerResourceType(getResourceTypeFor<CGameCoreSkeleton>());
  Resources.registerResourceType(getResourceTypeFor<CCurve>());
  Resources.registerResourceType(getResourceTypeFor<CTransCurve>());
  Resources.registerResourceType(getResourceTypeFor<CComputeShader>());
  Resources.registerResourceType(getResourceTypeFor<particles::TEmitter>());
}

void CEngine::start()
{
  PROFILE_FUNCTION("Engine::Start");
  _render = new CModuleRender("render");
  _gpu_culling = new CModuleGPUCulling();
  _entities = new CModuleEntities("entities");
  _physics = new CModulePhysics("physics");
  _cameraMixer = new CModuleCameraMixer("camera_mixer");
  _multithreading = new MultithreadingModule("multihtreading");
  _ui = new UI::CModuleUI("ui");
  _particles = new particles::CModuleParticles("particles");
  _gameController = new CModuleGameController("game_controller");
  _audioManager = new CModuleAudio("audio_manager");
  _scripting = new CModuleScripting("scripting");
  _fluidDecals = new CModuleFluidDecalGenerator("fluid_decals");
  _boot = new CModuleBoot("boot");
  _navmesh = new CModuleNavmesh("navmesh");
  _sceneManager = new CModuleSceneManager("scene_manager");

  {
    PROFILE_FUNCTION("Input");
    _input = new Input::CModuleInput("input_1");
    _input->registerDevice(new CDeviceKeyboard("keyboard"));
    _input->registerDevice(new CDeviceMouse("mouse"));
    _input->registerDevice(new CDevicePadXbox("gamepad", 0));
    _input->assignMapping("data/input/mapping.json");
  }

  
  {
    PROFILE_FUNCTION("Sys Modules");
    _modules.registerSystemModule(_navmesh);
    _modules.registerSystemModule(_render);
    _modules.registerSystemModule(_gpu_culling);
    _modules.registerSystemModule(_entities);
    _modules.registerSystemModule(_input);
    _modules.registerSystemModule(_physics);
    _modules.registerSystemModule(_cameraMixer);
    _modules.registerSystemModule(_multithreading);
    _modules.registerSystemModule(_gameController);
    _modules.registerSystemModule(_audioManager);
    _modules.registerSystemModule(_scripting);
    _modules.registerSystemModule(_ui);
    _modules.registerSystemModule(_particles);
	  _modules.registerSystemModule(_sceneManager);
  }

  _modules.registerGameModule(_boot);
  _modules.registerGameModule(new CModuleSplash("splash"));
  _modules.registerGameModule(new CModuleGameManager("game_manager"));
  _modules.registerGameModule(new CModuleMainMenu("main_menu"));
  _modules.registerGameModule(new CModuleGameplay("gameplay"));
  _modules.registerGameModule(new CModuleSampleObjs("sample_objs"));
  _modules.registerGameModule(new CModuleTestCameras("test_cameras"));
  _modules.registerGameModule(new CModuleCameraShaker("camera_shaker"));
  _modules.registerGameModule(new CModuleGamePaused("pause_menu"));
  _modules.registerGameModule(new CModuleGameUI("game_ui"));
  _modules.registerGameModule(new CModuleGameLoading("loading"));
  _modules.registerGameModule(new CModuleGameOver("gameover"));
  _modules.registerGameModule(new CModuleWinGame("wingame"));
  _modules.registerGameModule(new CModuleGameIntro("intro_game"));
  _modules.registerGameModule(_fluidDecals);

  _modules.start();

  dir_watcher_data.start("data", CApplication::get().getHandle());
}

void CEngine::stop()
{
  _modules.stop();
}

void CEngine::update(float dt)
{
  _modules.update(dt);
}

void CEngine::renderDebug()
{
  _modules.renderDebug();
}

CModuleManager& CEngine::getModules()
{
  return _modules;
}
