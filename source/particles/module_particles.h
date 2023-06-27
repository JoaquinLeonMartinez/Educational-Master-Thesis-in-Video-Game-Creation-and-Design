#pragma once

#include "modules/module.h"
#include "particles/particle.h"

class CMeshInstanced;

namespace particles
{
  struct TEmitter;
  class CSystem;

  // ----------------------------------------------
  class CModuleParticles : public IModule
  {
  public:
    CModuleParticles(const std::string& name);

    bool start() override;
    void update(float dt) override;

    CSystem* launchSystem(const TEmitter* emitter, CHandle owner);

    void renderInMenu() override;

  private:
    std::vector<CSystem*> _activeSystems;

    void registerEmitter(TEmitter* emitter);

    struct TRenderDetail {
      const TEmitter* emitter = nullptr;
      uint32_t        num_particles = 0;
      uint32_t        base = 0;
    };

    // Each instanced mesh type ( billboards, spheres, rocks ) rendered 
    // requires a CMeshInstance, and a uploadCall.
    struct TParticlesType {
      // Each vertex of this GPU mesh represents a particle when rendered
      CMeshInstanced*              mesh = nullptr;
      // Each emitter with have a group of the prev mesh assigned
      uint32_t                     num_emitters = 0;
      // Total particles accumulated during current frame
      uint32_t                     num_particles = 0;
      // This is the version in the cpu
      std::vector< TRenderData >   particles_cpu;
      std::vector< TRenderDetail > render_details;

      TRenderData* top() { return particles_cpu.data() + num_particles; }
      static const size_t max_particles_allowed_to_be_generated_system_and_frame = 256;
      TRenderData* max_top() { return particles_cpu.data() + particles_cpu.capacity() - max_particles_allowed_to_be_generated_system_and_frame; }
    };

    void uploadRenderDetailsToGPU();
    void saveRenderDetailsOfEmitter(const TEmitter* emitter, uint32_t nparticles);

    std::vector< TParticlesType > particles_types;

    // This is an entity in the scene to be able to register something in the 
    // render manager
    CHandle h_all_particles_entity;
    CHandle h_render;

  };
}
