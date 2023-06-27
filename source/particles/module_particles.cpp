#include "mcv_platform.h"
#include "particles/module_particles.h"
#include "particles/particle_system.h"
#include "particles/particle_emitter.h"
#include "render/meshes/mesh_instanced.h"
#include "components/common/comp_render.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_name.h"
#include "render/textures/material.h"

namespace particles
{
  CModuleParticles::CModuleParticles(const std::string& name)
    : IModule(name)
  {
  }

  bool CModuleParticles::start()
  {

    TParticlesType* pt = nullptr;
    particles_types.resize(2);
    
    // One for billboards
    pt = &particles_types[0];
    pt->mesh = (CMeshInstanced*)Resources.get("data/meshes/all_particles_billboards.instanced_mesh")->as<CMesh>();
    pt->particles_cpu.reserve(1024);
    
    // One for spheres
    pt = &particles_types[1];
    pt->mesh = (CMeshInstanced*)Resources.get("data/meshes/all_particles_spheres.instanced_mesh")->as<CMesh>();
    pt->particles_cpu.reserve(1024);

    // Create an empty entity with a transform and comp render
    // to be able to register it in the render manager
    h_all_particles_entity.create<CEntity>();
    CEntity* e_all_particles = h_all_particles_entity;
    assert(e_all_particles);

    CHandle h_name;
    TCompName* c_name = h_name.create<TCompName>();
    c_name->setName("All Particles");
    e_all_particles->set(h_name);

    CHandle h_transform;
    h_transform.create<TCompTransform>();
    e_all_particles->set(h_transform);

    h_render.create<TCompRender>();
    e_all_particles->set(h_render);

    assert(h_render.isValid());
    return true;
  }

  void CModuleParticles::update(float dt)
  {
    PROFILE_FUNCTION("particles.update");

    for (auto& pt : particles_types) {
      pt.render_details.clear();
      pt.num_particles = 0;
    }

    size_t max_particles_allowed_to_be_generated_system_and_frame = 256;

    size_t num_particles_reserved = 1024;

    // Current emitter 
    const TEmitter* curr_emitter = nullptr;

    // # particles accumulated for the current emitter, but not yet saved
    uint32_t nparticles = 0;

    // address to write next particle data, and a guard pointer
    TRenderData* out = nullptr;
    TRenderData* max_out = nullptr;
    
    for (auto& ps : _activeSystems)
    {
      PROFILE_FUNCTION("particle");

      // Are we changing of emitter? (We have our systems sorted by emitter type)
      const TEmitter* new_emitter = ps->_emitter;
      if (new_emitter != curr_emitter) {
        saveRenderDetailsOfEmitter(curr_emitter, nparticles);
        curr_emitter = new_emitter;
        nparticles = 0;

        // A change in the emitter can trigger a change in the particle type
        // storage
        TParticlesType& pt = particles_types[curr_emitter->particles_type_idx];
        out = pt.top();
        max_out = pt.max_top();
      }

      // Do we need to allocate more space as the next system might go beyond our buffer limits?
      if (out >= max_out ) {
        TParticlesType& pt = particles_types[curr_emitter->particles_type_idx];
        pt.particles_cpu.reserve(pt.particles_cpu.capacity() * 2);
        out = pt.top() + nparticles;
        max_out = pt.max_top();
      }

      TRenderData* new_out = ps->update(dt, out);
      assert(new_out >= out);
      assert(new_out <= max_out);

      uint32_t particles_added = (uint32_t)(new_out - out);
      nparticles += particles_added;

      out = new_out;
    }

    // Register the last group
    saveRenderDetailsOfEmitter(curr_emitter, nparticles);

    uploadRenderDetailsToGPU();
  }

  // -------------------------------------------------------------------------
  // register a call to render nparticles_per_emitter curr_emitter
  void CModuleParticles::saveRenderDetailsOfEmitter( const TEmitter* emitter, uint32_t nparticles ) {
    if (emitter && nparticles > 0) {
      // register a call to render nparticles_per_emitter curr_emitter
      TParticlesType& pt = particles_types[emitter->particles_type_idx];
      pt.render_details.push_back(TRenderDetail{ emitter, nparticles, pt.num_particles });
      pt.num_particles += nparticles;
    }
  }


  // -------------------------------------------------------------------------
  void CModuleParticles::uploadRenderDetailsToGPU( ) {
    PROFILE_FUNCTION("uploadRenderDetailsToGPU");

    // Update the render group of the mesh
    for (auto& pt : particles_types) {
      
      // Update GPU only if there is something to render
      if (pt.num_emitters > 0)
        pt.mesh->setInstancesData(pt.particles_cpu.data(), pt.num_particles, sizeof(TRenderData));

      VMeshGroups& groups = pt.mesh->getGroups();

      // Clear existing groups before updating the new values (only the existing groups are updated)
      for (auto& g : groups)
        g.num_indices = 0;

      // Take the offical mesh 
      for (auto& e : pt.render_details) {

        uint32_t render_group = e.emitter->render_group;
        assert(render_group < groups.size());

        // Update the render groups to match the resulting number of particles of each type
        groups[render_group].first_idx = e.base;
        groups[render_group].num_indices = e.num_particles;
      }
    }
  }

  void CModuleParticles::registerEmitter(TEmitter* emitter) {
    assert(emitter);
    assert(!emitter->isRegistered());
    assert(emitter->material);

    // Assign the first one: billboards
    assert(emitter->particles_type_idx < 2);

    TParticlesType& pt = particles_types[emitter->particles_type_idx];

    // Associate a unique index, which will be associated to a new 
    // render group of our mesh
    uint32_t mesh_group = pt.num_emitters;
    pt.num_emitters++;

    emitter->render_group = mesh_group;
    assert(emitter->isRegistered());

    // Add an empty group to the mesh. It will store how many particles
    // of this type we are rendering every frame. Right now: zero.
    VMeshGroups& groups = pt.mesh->getGroups();
    groups.push_back(TMeshGroup());

    // We will create a new fake RenderPart
    TCompRender::MeshPart part;
    part.mesh_instances_group = mesh_group;
    part.mesh = pt.mesh;
    part.material = emitter->material;

    TCompRender* c_render = h_render;
    assert(c_render);
    c_render->parts.push_back(part);

    // We will request the entity to register all renderpart in the rm
    c_render->updateRenderManager();
  }

  CSystem* CModuleParticles::launchSystem(const TEmitter* emitter, CHandle owner)
  {
    CSystem* ps = new CSystem(emitter);
    ps->setOwner(owner);
    ps->launch();

    _activeSystems.push_back(ps);

    // If the ps.emitter is new... we need to register in the render manager
    if (!emitter->isRegistered())
      // a render key that renders mesh + material + technique of the emitter
      registerEmitter((TEmitter*)emitter);

    // Sort by pointer of the resource, so sorting by type
    std::sort(_activeSystems.begin(), _activeSystems.end(), [](CSystem* a, CSystem* b) {
      return a->_emitter < b->_emitter;
      });

    return ps;
  }

  void CModuleParticles::renderInMenu()
  {
    if (ImGui::TreeNode("Particles...")) {

      // Sum total
      uint32_t total_particles = 0;
      uint32_t total_emitters = 0;
      for (auto& pt : particles_types) {
        total_emitters += pt.num_emitters;
        total_particles += pt.num_particles;
      }

      ImGui::Text("%ld active systems", _activeSystems.size());
      ImGui::Text("%d emitters", total_emitters);
      ImGui::Text("%d particles", total_particles);

      for (auto& pt : particles_types) {
        if (ImGui::TreeNode(pt.mesh->getName().c_str())) {
          ImGui::Text("%ld in %ld groups", pt.num_particles, pt.render_details.size());
          ImGui::Text("%d emitters", pt.num_emitters);
          for (auto& e : pt.render_details)
            ImGui::Text("x%d from %d of %s (slot %d)", e.num_particles, e.base, e.emitter->getName().c_str(), e.emitter->render_group );
          ImGui::TreePop();
        }
      }

      ImGui::TreePop();
    }

  }
}
