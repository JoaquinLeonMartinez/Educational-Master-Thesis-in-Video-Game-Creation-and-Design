#include "mcv_platform.h"
#include "render_manager.h"
#include "textures/material.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/common/comp_culling.h"
#include "components/common/comp_aabb.h"
#include "components/common/comp_num_instances.h"
#include "components/common/comp_buffers.h"
#include "skeleton/comp_skeleton.h"
#include "render/module_gpu_culling.h"
#include "engine.h"

static CRenderManager the_render_manager;
CRenderManager& CRenderManager::get() {
  return the_render_manager;
}

bool operator<(const CRenderManager::TNormalKey& k1, const eRenderCategory& category) {
  return k1.material->category < category;
}
bool operator<(const eRenderCategory& category, const CRenderManager::TNormalKey& k1) {
  return category < k1.material->category;
}

void CRenderManager::addKey(
  CHandle          h_owner,
  const CMesh* mesh,
  const CMaterial* material,
  CHandle          h_transform,
  uint16_t         submesh_id,
  uint16_t         instanced_group_id
) {

  assert(mesh);
  assert(material);

  TNormalKey key;
  key.mesh = mesh;
  key.material = material;
  key.h_transform = h_transform;
  key.h_owner = h_owner;
  key.submesh_id = submesh_id;
  key.instanced_group_id = instanced_group_id;
  key.is_instanced = 0;
  key.uses_custom_buffers = 0;
  normal_keys.add(key);

  // if material has shadows... add a shadowkey to the shadows_keys group
  if (material->castsShadows()) {
    TNormalKey key;
    key.mesh = mesh;
    key.material = material->getShadowsMaterial();
    assert(key.material);
    key.h_transform = h_transform;
    key.h_owner = h_owner;
    key.submesh_id = submesh_id;
    key.instanced_group_id = instanced_group_id;
    key.is_instanced = 0;
    key.uses_custom_buffers = 0;
    normal_keys.add(key);
  }

}

void CRenderManager::TNormalKey::renderInMenu() {

  char key_name[256];
  std::string mat_name = material->getName();
  snprintf(key_name, 255, "%s %s %s [%d]", material->getCategoryName(), mat_name.c_str(), mesh->getName().c_str(), submesh_id);
  if (ImGui::TreeNode(key_name)) {

    auto ncmaterial = const_cast<CMaterial*>(material);
    if (ImGui::TreeNode(material->getName().c_str())) {
      ncmaterial->renderInMenu();
      ImGui::TreePop();
    }

    auto ncmesh = const_cast<CMesh*>(mesh);
    ncmesh->renderInMenu();
    ImGui::TreePop();
  }
}

void CRenderManager::delKeys(CHandle h_transform) {
  normal_keys.del(h_transform);
}

bool CRenderManager::sortNormalKeys(const TNormalKey& k1, const TNormalKey& k2) {
  if (k1.material != k2.material) {
    if (k1.material->category != k2.material->category)
      return k1.material->category < k2.material->category;
    if (k1.material->priority != k2.material->priority)
      return k1.material->priority < k2.material->priority;
    // Compare using pointers... 
    return k1.material < k2.material;
  }
  if (k1.mesh != k2.mesh)
    return k1.mesh < k2.mesh;

  return k1.submesh_id < k2.submesh_id;
}

void CRenderManager::render(eRenderCategory category) {

  CGpuScope gpu_trace(category_names.nameOf(category));

  {
    PROFILE_FUNCTION("GPU Entities");
    Engine.getGPUCulling().renderCategory(category);
  }

  draw_calls_per_category[category] = 0;

  if (normal_keys.sort_required) {
    std::sort(normal_keys.keys.begin(), normal_keys.keys.end(), &sortNormalKeys);
    normal_keys.sort_required = false;

    for (auto& k : normal_keys.keys) {
      if (!k.h_transform.isValid()) {
        TCompRender* c_render = k.h_owner;
        k.h_transform = c_render->get<TCompTransform>();
        k.h_aabb = c_render->get<TCompAbsAABB>();
        k.is_instanced = c_render->get<TCompNumInstances>().isValid();
        k.uses_custom_buffers = c_render->get<TCompBuffers>().isValid();
      }

    }

  }

  // Find the limits of the category
  auto range = std::equal_range(
    normal_keys.keys.begin(),
    normal_keys.keys.end(),
    category
  );
  if (range.first == range.second)
    return;

  // Check if we have culling information from the camera source
  CEntity * e_camera = h_camera;
  const TCompCulling * culling = nullptr;
  if (e_camera)
    culling = e_camera->get<TCompCulling>();
  const TCompCulling::TCullingBits * culling_bits = culling ? &culling->bits : nullptr;

  // I want to use pointers (to avoid debug layout of VC on iterators)
  // but I can't dereference the end(), so use distance to get pointers
  auto offset_to_first = std::distance(normal_keys.keys.begin(), range.first);
  auto offset_to_last = std::distance(normal_keys.keys.begin(), range.second);

  const TNormalKey * it = normal_keys.keys.data() + offset_to_first;
  const TNormalKey * last = normal_keys.keys.data() + offset_to_last;

  static TNormalKey null_key;

  bool skin_active = false;

  uint32_t num_draw_calls = 0;
  const TNormalKey * prev_it = &null_key;
  while (it != last) {

    // Do the culling
    if (culling_bits) {
      TCompAbsAABB* aabb = it->h_aabb;
      if (aabb) {
        auto idx = it->h_aabb.getExternalIndex();
        if (!culling_bits->test(idx)) {
          ++it;
          continue;
        }
      }
    }

    if (it->material != prev_it->material) {
      if (prev_it != &null_key)
        CGpuTrace::pop();
      CGpuTrace::push(it->material->getName().c_str());
      it->material->activate();
      skin_active = it->material->tech->usesSkin();
    }

    if (it->mesh != prev_it->mesh)
      it->mesh->activate();

    if (it->h_transform != prev_it->h_transform) {
      TCompTransform* c_trans = it->h_transform;
      TCompRender* c_render = it->h_owner;
      activateObject(c_trans->asMatrix(), c_render->color);
    }

    if (skin_active) {
      const CEntity* e = it->h_owner.getOwner();
      if(e != nullptr) {//EVITAR PETE CUANDO SE REICICIA TRAS MORIR O QUERER IR AL ULTIMO CHECKPOINT
		  TCompSkeleton* skel = e->get<TCompSkeleton>();
		  skel->cb_bones.activate();
	  }
    }

    if (it->uses_custom_buffers) {
      const CEntity* e = it->h_owner.getOwner();
      TCompBuffers* c_buffers = e->get<TCompBuffers>();
      if (c_buffers)
        it->material->activateCompBuffers(c_buffers);
    }

    if (it->is_instanced) {
      CGpuTrace::setMarker(it->mesh->getName().c_str(), D3DCOLOR_XRGB(192, 128, 255));
      const CEntity* e = it->h_owner.getOwner();
      TCompNumInstances* c_num_instances = e->get<TCompNumInstances>();
      assert(c_num_instances);

      if (c_num_instances->is_indirect) {
        if (!c_num_instances->gpu_buffer) {
          TCompBuffers* c_buffers = e->get<TCompBuffers>();
          c_num_instances->gpu_buffer = c_buffers->getBufferByName(c_num_instances->gpu_buffer_name.c_str());
        }
        // 20 = sizeof(sizeof(DrawIndexedInstancedArgs)
        it->mesh->renderIndirect(c_num_instances->gpu_buffer, 20 * it->submesh_id);
      }
      else {
        if (c_num_instances->num_instances)
          it->mesh->renderGroupInstanced(it->submesh_id, c_num_instances->num_instances);
      }
    }
    else {
      CGpuTrace::setMarker(it->mesh->getName().c_str(), D3DCOLOR_XRGB(192, 255, 128));
      it->mesh->renderGroup(it->submesh_id, it->instanced_group_id);
    }

    prev_it = it;
    ++it;

    ++num_draw_calls;
  }

  if (prev_it != &null_key)
    CGpuTrace::pop();

  draw_calls_per_category[category] = num_draw_calls;
}

void CRenderManager::renderInMenu() {
  if (!ImGui::TreeNode("Render Manager"))
    return;

  if (ImGui::TreeNode("Stats")) {
    for (auto& it : draw_calls_per_category)
      ImGui::LabelText(category_names.nameOf(it.first), "%d", it.second);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Normal Keys")) {
    normal_keys.renderInMenu();
    ImGui::TreePop();
  }

  ImGui::TreePop();
}


