#pragma once
#include "modules/module.h"
#include "engine.h"

class Scene {//de momento cargamos navmesh de una escena (de un mapa), mas tarde se cargara prefabs de una escena

public:
	CHandle h_scene;

	std::string name;
	std::string navmesh;
	//std::vector<std::string> prefabs;
};


class CModuleSceneManager : public IModule
{
public:
  CModuleSceneManager(const std::string& name);
  bool start() override;
  void stop() override;

  bool removeActiveScene();
  bool loadScene(const std::string & name);
  Scene* newScene(const std::string& name);
  void loadJsonScenes(const std::string filepath);
  CModuleSceneManager* getSceneManager() { return this; }
  //------------hasta aqui

private:
  //bool sceneInCurse;
  void renderInMenu();
  Scene* sceneInUse;
  //NUEVO
  std::map<std::string, Scene*> scenes;
};


