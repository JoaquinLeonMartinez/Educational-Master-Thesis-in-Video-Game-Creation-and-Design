#pragma once

#include "modules/module.h"

class CMeshInstanced;

class CModuleTestInstancing : public IModule {

  // -------------------------------------------------------------------
  struct TSphereInstance {
    MAT44 world;
  };
  CMeshInstanced* sphere_instances_mesh = nullptr;
  std::vector< TSphereInstance > sphere_instances;

  // -------------------------------------------------------------------
  struct TInstanceBlood {
    MAT44 world;
    VEC4  color;
  };
  CMeshInstanced* blood_instances_mesh = nullptr;
  std::vector< TInstanceBlood > blood_instances;

  // -------------------------------------------------------------------
public:
  CModuleTestInstancing(const std::string& name);
  bool start() override;
  void renderDebug() override;
  void renderInMenu() override;
  void update(float dt) override;
};