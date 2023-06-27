#include "mcv_platform.h"
#include "module_scenes.h"
#include "engine.h"
#include "entity/entity_parser.h"
#include "modules/module_navmesh.h"

CModuleSceneManager::CModuleSceneManager(const std::string& name)
  : IModule(name)
{}

bool CModuleSceneManager::start()
{
	
	sceneInUse = newScene("INIT");//se carga una escena inicial
	loadJsonScenes("data/bootScenes.json");
	return true;
}

//metodos Nuevos, cargar escena, de momento navmesh
bool CModuleSceneManager::loadScene(const std::string & name) {
	auto it = scenes.find(name);

	
	if (it != scenes.end())
	{
		removeActiveScene();
		Scene * current_scene = it->second;
		Engine.getNavmesh().createNavmesh(current_scene->navmesh);
		sceneInUse = current_scene;
		return true;
	}
	return false;
}
//creacion de escena
Scene* CModuleSceneManager::newScene(const std::string& name) {

	Scene* scene = new Scene();
	scene->name = name;
	scene->navmesh = "";

	return scene;
}
//precarga de escena
void CModuleSceneManager::loadJsonScenes(const std::string filepath) {
	json jboot = loadJson(filepath);
	for (auto it = jboot.begin(); it != jboot.end(); ++it) {

	
		std::string scene_name = it.key();
		
		Scene* scene = newScene(scene_name);
		std::string navmesh_name = jboot[scene_name]["navmesh"];
		scene->navmesh = navmesh_name;
		//mas cosas de prefabs de escenas
		

		scenes.insert(std::pair<std::string, Scene*>(scene_name, scene));
	}
}


void CModuleSceneManager::stop() {

	removeActiveScene();

}

void CModuleSceneManager::renderInMenu() {
	if (ImGui::TreeNode("Scene Manager")) {
		if (ImGui::TreeNode("Name and Navmesh")) {
			for (auto scene : scenes)
				ImGui::Text("Nombre: %s Navmesh: %s", scene.second->name.c_str(), scene.second->navmesh.c_str());
			ImGui::TreePop();

		}
		
		ImGui::Text("Escena Actual: %s", sceneInUse->name.c_str());
		
		ImGui::TreePop();
	}
	

}

bool CModuleSceneManager::removeActiveScene() {//borramos cosas
	if (sceneInUse != nullptr) {
		
		
		EngineNavmesh.destroyNavmesh();
		sceneInUse = nullptr;
		return true;
	}

	return false;
}


