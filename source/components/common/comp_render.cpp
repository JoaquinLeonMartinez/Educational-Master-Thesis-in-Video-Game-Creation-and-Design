#include "mcv_platform.h"
#include "comp_render.h"

#include "render/render.h"
#include "render/textures/material.h"
#include "render/meshes/mesh.h"
#include "render/render_manager.h"

#include "components/common/comp_transform.h"
#include "skeleton/comp_skeleton.h"

DECL_OBJ_MANAGER("render", TCompRender);

using VStrings = std::vector< std::string >;

TCompRender::~TCompRender() {
  CRenderManager::get().delKeys(CHandle(this));
}

// --------------------------------------------
void TCompRender::onDefineLocalAABB(const TMsgDefineLocalAABB& msg) {
  AABB::CreateMerged(*msg.aabb, *msg.aabb, aabb);
}

void TCompRender::registerMsgs() {
  DECL_MSG(TCompRender, TMsgDefineLocalAABB, onDefineLocalAABB);
}

void TCompRender::debugInMenu() {
  ImGui::ColorEdit4("Color", &color.x);
  bool changed = ImGui::Checkbox("Entity Visible", &is_visible);;
  for (auto& p : parts) {
    ImGui::PushID(&p);
    ImGui::LabelText("Group", "%d", p.mesh_group);
    ImGui::LabelText("Mesh", p.mesh->getName().c_str());
    ImGui::LabelText("State", "%d", p.state);
    changed |= ImGui::Checkbox("Visible", &p.is_visible);
    if (ImGui::TreeNode("Material")) {
      ((CMaterial*) p.material)->renderInMenu();
      ImGui::TreePop();
    }
    ImGui::PopID();
  }

  if (ImGui::DragInt("Mesh State", &curr_state, 0.02f, 0, 4))
    showMeshesWithState(curr_state);

  ImGui::DragFloat3("AABB Center", &aabb.Center.x, 0.01f, -5.0f, 5.0f);
  ImGui::DragFloat3("AABB Half", &aabb.Extents.x, 0.01f, -5.0f, 5.0f);

  if (changed)
    updateRenderManager();
}

void TCompRender::readMesh(const json& j) {
  MeshPart mp;

  if (!j.count("mesh")) {
    fatal("Missing attribute mesh reading mesh in input json %s", j.dump(2, ' ').c_str());
  }

  std::string mesh_name = j.value("mesh", "data/meshes/Teapot001.mesh");
  mp.mesh = Resources.get(mesh_name)->as<CMesh>();

  //dbg("For mesh %s\n", mesh_name.c_str());
  int mesh_instances_group = j.value("instances_group", 0);

  if (j.count("materials")) {
    VStrings names = j["materials"].get< VStrings >();
    int idx = 0;
    for (auto mat_name : names) {
      mp.material = Resources.get(mat_name)->as<CMaterial>();
      mp.mesh_group = idx;
      mp.mesh_instances_group = mesh_instances_group;
      mp.is_visible = j.value("visible", true);
      mp.state = j.value("state", 0);
      //dbg("Slot %d will use mat %s\n", idx, mat_name.c_str());
      ++idx;
      parts.push_back(mp);
    }
  }
  else {
    std::string mat_name = j.value("material", "data/materials/wood.material");
    mp.material = Resources.get(mat_name)->as<CMaterial>();
    mp.mesh_group = 0;
    mp.mesh_instances_group = mesh_instances_group;
    mp.is_visible = j.value("visible", true);
    mp.state = j.value("state", 0);
    parts.push_back(mp);
  }

  AABB::CreateMerged(aabb, aabb, mp.mesh->getAABB());
}

void TCompRender::load(const json& j, TEntityParseContext& ctx) {
  if (j.count("color"))
    color = loadColor(j, "color");
  else
    color = VEC4(1, 1, 1, 1);

  if (j.count("meshes")) {
    auto& jmeshes = j["meshes"];
    assert(jmeshes.is_array());
    for( int i=0; i<jmeshes.size(); ++i )
      readMesh(jmeshes[i]);
  }
  else {
    readMesh(j);
  }
  if(j.count("is_visible"))
    is_visible = j.value("is_visible",is_visible);

  updateRenderManager();
}

void TCompRender::showMeshesWithState(int new_state) {
  curr_state = new_state;
  for (auto& p : parts)
    p.is_visible = (p.state == new_state);
  updateRenderManager();
}

void TCompRender::updateRenderManager() {
  CRenderManager& rm = CRenderManager::get();
  CHandle h_me = CHandle(this);

  // First, delete all references from me in the RenderManager
  rm.delKeys(CHandle(this));

  // Then register all draw calls which are active
  for (auto& p : parts) {
    if (!p.is_visible || !is_visible)
      continue;
    rm.addKey(
      h_me, p.mesh, p.material, CHandle(), p.mesh_group, p.mesh_instances_group
    );
  }

}

void TCompRender::renderDebug() {

  TCompTransform* transform = get<TCompTransform>();
  MAT44 world = transform->asMatrix();

  TCompSkeleton* c_skel = get<TCompSkeleton>();
  if (c_skel) {
    c_skel->updateCtesBones();
    c_skel->cb_bones.activate();
  }

  VEC4 color = VEC4(1, 1, 1, 1);
  activateObject(world, color);

  for (auto& p : parts) {
    if (!p.is_visible || !is_visible)
      continue;
    activateDebugTech(p.mesh);
    p.mesh->activate();
    p.mesh->renderGroup(p.mesh_group, p.mesh_instances_group);
  }

}

void TCompRender::setMaterial(const std::string &name, bool unMaterial) {
	CMaterial * material = (CMaterial*)Resources.get(name)->as<CMaterial>();
	if(unMaterial) {
		for (auto& p : parts) {
			if (!p.is_visible || !is_visible){
				continue;
			}
			p.material = material;
		}
	}
	else {
		for (int i = 0; i < parts.size(); i++) {
			if (i == 0) {
				if (!parts[i].is_visible || !parts[i].is_visible) {
					continue;
				}
				parts[i].material = material;
			}
		}
	}
	updateRenderManager();
}

