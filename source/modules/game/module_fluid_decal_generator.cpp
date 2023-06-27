#include "mcv_platform.h"
#include "module_fluid_decal_generator.h"

#include "render/meshes/mesh_instanced.h"
#include "engine.h"
#include "modules/module_physics.h"

using namespace physx;

float CModuleFluidDecalGenerator::unitRandom() {
    return (float)rand() / (float)RAND_MAX;
}

float CModuleFluidDecalGenerator::randomFloat(float vmin, float vmax) {
    return vmin + (vmax - vmin) * unitRandom();
}

CModuleFluidDecalGenerator::CModuleFluidDecalGenerator(const std::string& name)
    : IModule(name)
{}

bool CModuleFluidDecalGenerator::start() 
{
    // -----------------------
    {
        auto rmesh = Resources.get("data/meshes/blood.instanced_mesh")->as<CMesh>();
        blood_instances_mesh = (CMeshInstanced*)rmesh;
    }

    return true;
}

void CModuleFluidDecalGenerator::update(float dt)
{
  actualTime += dt;
  if (actualTime > removalPeriod) 
  {
    actualTime = 0.f;
    for (int i = 0; i < blood_instances.size(); i++) {
      CModuleFluidDecalGenerator::TInstanceBlood instance = blood_instances.at(i);
      if(instance.time_stamp + instance.time_transition < Time.current){
        blood_instances.erase(blood_instances.begin() + i);
        i--;
      }
    }

  }

  // Upload blood instances
  blood_instances_mesh->setInstancesData(blood_instances.data(), blood_instances.size(), sizeof(TInstanceBlood));
}

void CModuleFluidDecalGenerator::renderInMenu() {

    // ----------------------------------------------
    if (ImGui::TreeNode("Fluids")) {
        static int num = 3;
        static float sz = 2.0f;
        ImGui::DragFloat("Size", &sz, 0.01f, -50.f, 50.f);
        ImGui::DragInt("Num", &num, 0.1f, 1, 10);
        ImGui::Text("Num Instances: %ld / %ld", blood_instances.size(), blood_instances.capacity());
        if (ImGui::Button("Add")) {
            for (int i = 0; i < num; ++i) {
                TInstanceBlood new_instance;
                new_instance.world =
                    MAT44::CreateRotationY(randomFloat((float)-M_PI, (float)M_PI))
                    *
                    MAT44::CreateScale(randomFloat(0.25f, 0.75f))
                    *
                    MAT44::CreateTranslation(VEC3(randomFloat(-sz, sz), 0, randomFloat(-sz, sz)));
                new_instance.color.x = unitRandom();
                new_instance.color.y = unitRandom();
                new_instance.color.z = 1 - new_instance.color.x - new_instance.color.y;
                if (new_instance.color.z < 0.f) new_instance.color.z = 0.0f;
                new_instance.color.w = 1;
                new_instance.time_stamp = Time.current + 5.f; //5 seconds to disappear
                blood_instances.push_back(new_instance);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Del") && !blood_instances.empty())
            blood_instances.pop_back();
        if (ImGui::TreeNode("Instances")) {
            for (auto& p : blood_instances) {
                ImGui::PushID(&p);
                VEC3 scale, trans;
                QUAT rot;
                p.world.Decompose(scale, rot, trans);
                CTransform tmx;
                tmx.setRotation(rot);
                tmx.setPosition(trans);
                tmx.setScale(scale.x);
                if (tmx.renderInMenu())
                    p.world = tmx.asMatrix();
                ImGui::ColorEdit4("Color", &p.color.x);
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

void CModuleFluidDecalGenerator::generateFluid(int impactForce, VEC3 position, VEC3 normal) {//EXTEND THE GENERATION TO MORE THAN PLANES USING NORMAL
    int num = impactForce / 2;
    int radius = impactForce / 8;
    for (int i = 0; i < num; ++i) {
        TInstanceBlood new_instance;

        VEC3 desiredPosition = position;
        desiredPosition.y += 0.001f; //So we can detect collision with planes
        physx::PxRaycastHit hit;
        physx::PxFilterData pxFilterData;
        pxFilterData.word0 = EnginePhysics.Scenario;
        physx::PxQueryFilterData PxScenarioFilterData;
        PxScenarioFilterData.data = pxFilterData;
        PxScenarioFilterData.flags = physx::PxQueryFlag::eSTATIC;
        EnginePhysics.Raycast(desiredPosition, VEC3(0, -1, 0), 30.f, hit, physx::PxQueryFlag::eSTATIC, PxScenarioFilterData);
        if (hit.position != physx::PxVec3(0)) {
          desiredPosition = PXVEC3_TO_VEC3(hit.position);
        }

        new_instance.world =
            MAT44::CreateRotationY(randomFloat((float)-M_PI, (float)M_PI))
            *
            MAT44::CreateScale(randomFloat(0.1f, 0.75f))
            *
            MAT44::CreateTranslation(desiredPosition + VEC3(randomFloat(-radius, radius), 0, randomFloat(-radius, radius)));
        new_instance.color.x = unitRandom();
        new_instance.color.y = unitRandom();
        new_instance.color.z = 1 - new_instance.color.x - new_instance.color.y;
        if (new_instance.color.z < 0.f) new_instance.color.z = 0.0f;
        new_instance.color.w = 1;
        new_instance.time_stamp = Time.current + 5.f; //5 seconds to disappear
        blood_instances.push_back(new_instance);
    }
}

void CModuleFluidDecalGenerator::generateSingleFluidUncapped(float scale, VEC3 exactPosition, VEC3 normal) {//EXTEND THE GENERATION TO MORE THAN PLANES USING NORMAL
    TInstanceBlood new_instance;

    VEC3 desiredPosition = exactPosition;
    desiredPosition.y += 0.001f; //So we can detect collision with planes
    physx::PxRaycastHit hit;
    physx::PxFilterData pxFilterData;
    pxFilterData.word0 = EnginePhysics.Scenario;
    physx::PxQueryFilterData PxScenarioFilterData;
    PxScenarioFilterData.data = pxFilterData;
    PxScenarioFilterData.flags = physx::PxQueryFlag::eSTATIC;
    EnginePhysics.Raycast(desiredPosition, VEC3(0, -1, 0), 30.f, hit, physx::PxQueryFlag::eSTATIC, PxScenarioFilterData);
    if (hit.position != physx::PxVec3(0)) {
      desiredPosition = PXVEC3_TO_VEC3(hit.position);
    }

    new_instance.world =
        MAT44::CreateRotationY(randomFloat((float)-M_PI, (float)M_PI))
        *
        MAT44::CreateScale(scale)
        *
        MAT44::CreateTranslation(desiredPosition);
    new_instance.color.x = unitRandom();
    new_instance.color.y = unitRandom();
    new_instance.color.z = 1 - new_instance.color.x - new_instance.color.y;
    if (new_instance.color.z < 0.f) new_instance.color.z = 0.0f;
    new_instance.color.w = 1;
    new_instance.time_stamp = Time.current + 5.f; //5 seconds to disappear
    blood_instances.push_back(new_instance);
}

void CModuleFluidDecalGenerator::renderDebug() {
}
