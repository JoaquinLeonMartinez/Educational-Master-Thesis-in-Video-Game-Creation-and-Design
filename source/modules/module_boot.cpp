#include "mcv_platform.h"
#include "module_boot.h"
#include "engine.h"
#include "entity/entity_parser.h"
#include "ui/module_ui.h"
CModuleBoot::CModuleBoot(const std::string& name)
  : IModule(name)
{}

bool CModuleBoot::start()
{
	

  json json = loadJson("data/boot.json");
  auto prefabs = json["scenes_to_load"].get< std::vector< std::string > >();
  CEngine::get().getGPUCulling().createPrefabProducts();
  for (auto& p : prefabs) {
    TFileContext fc(p);
    TEntityParseContext ctx;
    PROFILE_FUNCTION_COPY_TEXT( p.c_str() );
    dbg("Parsing boot prefab %s\n", p.c_str());
    //Instead of parsing it with parseScene, lets parse it with the GPU Culling Module
    //parseScene(p, ctx);
    //This only parses the entities and prefabs, not products
    CEngine::get().getGPUCulling().parseEntities(p, ctx);

  }
  
  //this makes the fetchresults from physx to be faster, but.. doesnt looks like
  EnginePhysics.gScene->forceDynamicTreeRebuild(true, true);
  CEngine::get().getGPUCulling().clear();

   prefabs = json["prefabs_to_prefaload"].get< std::vector< std::string > >();
  for (auto& p : prefabs) {
	  TFileContext fc(p);
	  TEntityParseContext ctx;
	  PROFILE_FUNCTION_COPY_TEXT(p.c_str());
	  dbg("Parsing boot prefab %s\n", p.c_str());
	  parseScene(p, ctx);
	  for (auto h : ctx.entities_loaded)
		  h.destroy();
  }

  CHandleManager::destroyAllPendingObjects();

  Time.loadedFrame();

  isLoadAll = true;


  return true;
}

