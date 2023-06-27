#include "mcv_platform.h"
#include "resource.h"

CResourcesManager Resources;

void IResource::setNameAndType(const std::string& new_name, const CResourceType* new_resource_type) {
  assert(!new_name.empty());
  assert(name.empty());
  assert(resource_type == nullptr);
  assert(new_resource_type != nullptr);
  name = new_name;
  resource_type = new_resource_type;
}

void CResourcesManager::onFileChanged(const std::string& filename) {
  for (auto it : all_resources)
    it.second->onFileChanged(filename);
}


void CResourcesManager::renderInMenu() {
  if (!ImGui::TreeNode("Resources"))
    return;

  for (auto rt : resource_types) {
    if (ImGui::TreeNode(rt->getName())) {
      for (auto it : all_resources) {
        IResource* res = it.second;

        // Is of the current resource type I'm showing...
        if (res->getResourceType() == rt) {

          // Resource name
          if (ImGui::TreeNode(it.first.c_str())) {
            res->renderInMenu();
            ImGui::TreePop();
          }

        }
      }
      ImGui::TreePop();
    }
  }

  ImGui::TreePop();
}

void CResourcesManager::destroyAll() {
  // Delete all resources
  for (auto it : all_resources) 
    delete it.second;
  all_resources.clear();
}

