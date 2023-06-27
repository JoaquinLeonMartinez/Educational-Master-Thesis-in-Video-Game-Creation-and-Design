#include "mcv_platform.h"
#include "module_test_instancing.h"
#include "render/meshes/mesh_instanced.h"


void renderMATT44InMenu( MAT44& world) {
  VEC3 scale, trans;
  QUAT rot;
  world.Decompose(scale, rot, trans);
  CTransform tmx;
  tmx.setRotation(rot);
  tmx.setPosition(trans);
  tmx.setScale(scale.x);
  if (tmx.renderInMenu())
    world = tmx.asMatrix();
}

CModuleTestInstancing::CModuleTestInstancing(const std::string& name)
  : IModule(name)
{}

bool CModuleTestInstancing::start() {
  {
    sphere_instances_mesh = (CMeshInstanced*) Resources.get("data/meshes/figures_GeoSphere002.instanced_mesh")->as<CMesh>();
    // Split instances in two groups: 4 + 6
    VMeshGroups groups;
    groups.resize(2);
    groups[0].first_idx = 0;
    groups[0].num_indices = 4;
    groups[1].first_idx = 4;
    groups[1].num_indices = 6;
    sphere_instances_mesh->setGroups(groups);
    sphere_instances.resize(10);
    for (int i = 0; i < (int)sphere_instances.size(); ++i)
      sphere_instances[i].world = MAT44::CreateTranslation(VEC3((float)i * 6.0f, (float)i, -10.0f));
  }

  // -----------------------
  {
    auto rmesh = Resources.get("data/meshes/blood.instanced_mesh")->as<CMesh>();
    blood_instances_mesh = (CMeshInstanced*)rmesh;
  }

  return true;
}

void CModuleTestInstancing::update(float dt)
{

  // Move the sphere instances
  {
    static float curr_time = 0.f;
    int idx = 0;
    for (auto & it : sphere_instances) {
      it.world = MAT44::CreateTranslation(VEC3((float)idx * 6.0f, 5.0f + 2.0f * sinf(curr_time + idx * 2.f), -10.0f));
      ++idx;
    }
    curr_time += Time.delta;

    // Upload sphere instances
    sphere_instances_mesh->setInstancesData(sphere_instances.data(), sphere_instances.size(), sizeof(TSphereInstance));
  }

  // Upload blood instances
  blood_instances_mesh->setInstancesData(blood_instances.data(), blood_instances.size(), sizeof(TInstanceBlood));
}

void CModuleTestInstancing::renderInMenu() {
  if (ImGui::TreeNode("Sphere Instances")) {
    for (auto& p : sphere_instances) {
      ImGui::PushID(&p);
      renderMATT44InMenu(p.world);
      ImGui::PopID();
    }
    ImGui::TreePop();
  }

  // ----------------------------------------------
  if (ImGui::TreeNode("Blood")) {
    static int num = 3;
    static float sz = 30.0f;
    ImGui::DragFloat("Size", &sz, 0.01f, -50.f, 50.f);
    ImGui::DragInt("Num", &num, 0.1f, 1, 10);
    ImGui::Text("Num Instances: %ld / %ld", blood_instances.size(), blood_instances.capacity());
    if (ImGui::Button("Add")) {
      for (int i = 0; i < num; ++i) {
        TInstanceBlood new_instance;
        new_instance.world =
          MAT44::CreateRotationY(randomFloat((float)-M_PI, (float)M_PI))
          *
          MAT44::CreateScale(randomFloat(2.f, 10.f))
          *
          MAT44::CreateTranslation(VEC3(randomFloat(-sz, sz), 0, randomFloat(-sz, sz)));
        new_instance.color.x = unitRandom();
        new_instance.color.y = unitRandom();
        new_instance.color.z = 1 - new_instance.color.x - new_instance.color.y;
        if (new_instance.color.z < 0.f) new_instance.color.z = 0.0f;
        new_instance.color.w = 1;
        blood_instances.push_back(new_instance);
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Del") && !blood_instances.empty())
      blood_instances.pop_back();
    if (ImGui::TreeNode("Instances")) {
      for (auto& p : blood_instances) {
        ImGui::PushID(&p);
        renderMATT44InMenu(p.world);
        ImGui::ColorEdit4("Color", &p.color.x);
        ImGui::PopID();
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }


}

void CModuleTestInstancing::renderDebug() {
}
