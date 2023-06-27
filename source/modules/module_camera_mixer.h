#pragma once

#include "modules/module.h"

//class IInterpolator;

class CModuleCameraMixer : public IModule
{
public:
  CModuleCameraMixer(const std::string& aname) : IModule(aname) { }
  bool start() override;
  void stop() override;
  void update(float delta) override;
  void renderDebug() override;
  void renderInMenu() override;

  void blendCamera(CHandle camera, float blendTime, Interpolator::IInterpolator* interpolation = nullptr);
  void setDefaultCamera(CHandle camera) { _defaultCamera = camera; }
  void setOutputCamera(CHandle camera) { _outputCamera = camera; }
  
  float getCameraWeight(CHandle c) const;

  Interpolator::IInterpolator * getInterpolator(std::string interpolator);

  CCamera saved_result;

private:
  void blendCameras(const CCamera* camera1, const CCamera* camera2, float ratio, CCamera& output);

  struct TMixedCamera
  {
    CHandle camera;
    float blendTime = 0.f;
    float targetWeight = 1.f; // to be controlled from outside
    float blendedWeight = 0.f;
    float appliedWeight = 0.f;
    Interpolator::IInterpolator* interpolator = nullptr;
  };

  std::vector<TMixedCamera> _mixedCameras;
  CHandle _defaultCamera;
  CHandle _outputCamera;
};
