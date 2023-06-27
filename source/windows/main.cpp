#include "mcv_platform.h"
#include "render/render.h"
#include "windows/app.h"
#include "render/primitives.h"
#include "render/module_render.h"
#include "engine.h"
#include "utils/file_context.h"


//std::set< void* > allocs;

/*void * operator new(size_t size)
{
  void * p malloc(size);
  if(size > 1024*1024){
    if(TFileContext::files.empty())
      dbg("Alloc of %ld bytes --\n",size);
    else
      dbg("Alloc of %ld bytes %s\n", size, TFileContext::files.back()->c_str());
      //allocs.insert(p);
  }
  return p;
}

void operator delete(void * p){
  free(p);
}*/


void CApplication::generateFrame() {
  PROFILE_FRAME_BEGINS();
  PROFILE_FUNCTION("App::generateFrame");

  // This allow use of Imgui anytime during update/render
  auto& mod_render = CEngine::get().getRender();
  //mod_render.beginImgui();

  // Update
  float elapsed = time_since_last_render.elapsedAndReset();
  Time.set(Time.current + elapsed);
  CEngine::get().update(Time.delta);

  // Render
  mod_render.generateFrame();

  // The last thing we do is render imgui menus
  //mod_render.endImgui();
  Render.swapChain();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
  PROFILE_SET_NFRAMES(3);
  PROFILE_FRAME_BEGINS();

  Remotery* rmt;
  rmt_CreateGlobalInstance(&rmt);

  json cfg = loadJson("data/config.json");
  const json& cfg_render = cfg["render"];
  int render_width = cfg_render.value("width", 1280);
  int render_height = cfg_render.value("height", 800);

  CApplication app;
  if (!app.create(hInstance, nCmdShow, render_width, render_height))
    return -1;

  if (!Render.create( app.getHandle(), render_width, render_height))
    return -2;

  CEngine::get().registerResourceTypes();

  app.runMainLoop();

  Render.destroy();

  rmt_DestroyGlobalInstance(rmt);

	return 0;
}