#include "mcv_platform.h"
#include "module_camera_mixer.h"
#include "components/common/comp_camera.h"

namespace
{
  const char* getCameraName(CHandle hCamera)
  {
    if (hCamera.isValid())
    {
      CEntity* e = hCamera;
      return e->getName();
    }
    return "";
  }

  CCamera* getCameraFromHandle(CHandle hCamera)
  {
    if (hCamera.isValid())
    {
      CEntity* e = hCamera;
      TCompCamera* cCamera = e->get<TCompCamera>();
      if (cCamera)
      {
        return cCamera;
      }
    }
    return nullptr;
  }

  void renderInterpolator(const char* name, Interpolator::IInterpolator* interpolator)
  {
    const int nsamples = 50;
    float values[nsamples];
    for (int i = 0; i < nsamples; ++i)
    {
      values[i] = interpolator->blend(0.f, 1.f, (float)i / (float)nsamples);
    }
    ImGui::PlotLines(name, values, nsamples, 0, 0,
      std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
      ImVec2(150, 80));
  }
}

bool CModuleCameraMixer::start()
{
  return true;
}

void CModuleCameraMixer::stop()
{

}

void CModuleCameraMixer::update(float delta)
{
  delta = Time.delta_unscaled;
  // update mixed cameras
  float weight = 1.f;
  for (auto it = _mixedCameras.rbegin(); it != _mixedCameras.rend(); ++it)
  {
    TMixedCamera& mc = *it;

    if (mc.blendedWeight < 1.f)
    {
      mc.blendedWeight = clamp(mc.blendedWeight + delta / mc.blendTime, 0.f, 1.f);
    }

    mc.appliedWeight = mc.blendedWeight * std::min(mc.targetWeight, weight);
    weight -= mc.appliedWeight;
  }

  // erase dead cameras
  auto endIt = std::remove_if(_mixedCameras.begin(), _mixedCameras.end(), [](const TMixedCamera& mc) {
    return mc.appliedWeight <= 0.f;
  });
  _mixedCameras.erase(endIt, _mixedCameras.end());
  //SEND MESSAGE HERE TO DESTROY IT

  
  // alternative erase method -> do not delete this code
  //for (auto it = _mixedCameras.begin(); it != _mixedCameras.end();)
  //{
  //  if (it->appliedWeight <= 0.f)
  //    it = _mixedCameras.erase(it);
  //  else
  //    ++it;
  //}

  // blend all active cameras
  CCamera result;

  CCamera* defaultCamera = getCameraFromHandle(_defaultCamera);
  if (defaultCamera)
  {
    result = *defaultCamera;
  }

  for (auto& mc : _mixedCameras)
  {
    CCamera* camera = getCameraFromHandle(mc.camera);

    float ratio = mc.blendedWeight;
    if (mc.interpolator)
    {
      ratio = mc.interpolator->blend(0.f, 1.f, ratio);
    }

    if (isnan(ratio))
      ratio = 0.f;
    blendCameras(&result, camera, ratio, result);
  }

  CCamera* outputCamera = getCameraFromHandle(_outputCamera);
  if (outputCamera)
  {
    *outputCamera = result;
    saved_result = result;
  }
}

float CModuleCameraMixer::getCameraWeight(CHandle c) const{
  for (auto& mc : _mixedCameras)
  {
    if (mc.camera == c) {
      return mc.appliedWeight;
    }
  }
  return -1.f;
}

void CModuleCameraMixer::blendCameras(const CCamera* camera1, 
                                      const CCamera* camera2,
                                      float ratio, 
                                      CCamera& output)
{
  if (!camera1 || !camera2 || ratio <= 0.f)
    return;

  const VEC3 newPosition = VEC3::Lerp(camera1->getPosition(), camera2->getPosition(), ratio);
  const VEC3 newFront = VEC3::Lerp(camera1->getFront(), camera2->getFront(), ratio);
  const VEC3 newUp = VEC3::Lerp(camera1->getUp(), camera2->getUp(), ratio);
  const float newFov = camera1->getFov() * (1.f-ratio) + camera2->getFov() * ratio;
  const float newZNear = camera1->getNear() * (1.f - ratio) + camera2->getNear() * ratio;
  const float newZFar = camera1->getFar() * (1.f - ratio) + camera2->getFar() * ratio;

  output.setProjectionParams(newFov, newZNear, newZFar);
  output.setViewport(0,0,Render.width, Render.height);
  output.lookAt(newPosition, newPosition + newFront, newUp);
}

void CModuleCameraMixer::blendCamera(CHandle camera, float blendTime, Interpolator::IInterpolator* interpolation)
{
  TMixedCamera mc;
  mc.camera = camera;
  mc.blendTime = blendTime;
  mc.interpolator = interpolation;
  mc.blendedWeight = 0.f;
  mc.appliedWeight = 0.f;

  _mixedCameras.push_back(std::move(mc));
}



Interpolator::IInterpolator* CModuleCameraMixer::getInterpolator(std::string interpolator) {
	if (interpolator.compare("linear") == 0) {
		static Interpolator::TLinearInterpolator linear;
		return &linear;
	}
	else if (interpolator.compare("Quadin") == 0) {
		static Interpolator::TQuadInInterpolator quadin;
		return &quadin;
	}
	else if (interpolator.compare("Quadout") == 0) {
		static Interpolator::TQuadOutInterpolator quadout;
		return &quadout;
	}
	else if (interpolator.compare("Quadinout") == 0) {
		static Interpolator::TQuadInOutInterpolator quadinout;
		return &quadinout;
	}
	else if (interpolator.compare("Cubicin") == 0) {
		static Interpolator::TCubicInInterpolator cubicin;
		return &cubicin;
	}
	else if (interpolator.compare("Cubicout") == 0) {
		static Interpolator::TCubicOutInterpolator cubicout;
		return &cubicout;
	}
	else if (interpolator.compare("Cubicinout") == 0) {
		static Interpolator::TCubicInOutInterpolator cubicinout;
		return &cubicinout;
	}
	
	return nullptr;
}

void CModuleCameraMixer::renderDebug()
{

}

void CModuleCameraMixer::renderInMenu()
{
  using namespace ImGui;

  if (TreeNode("camera_mixer"))
  {
    Text("Default camera: %s", getCameraName(_defaultCamera));
    Text("Output camera: %s", getCameraName(_outputCamera));

    Separator();
    Columns(4);

    for (auto& mc : _mixedCameras)
    {
      Text(getCameraName(mc.camera));           NextColumn();
      Text("%.0f%%", mc.targetWeight * 100.f);  NextColumn();
      ProgressBar(mc.blendedWeight);            NextColumn();
      ProgressBar(mc.appliedWeight);            NextColumn();
    }

    Columns(1);
    TreePop();
  }

  if (ImGui::TreeNode("Interpolators"))
  {
    renderInterpolator("Linear", &Interpolator::TLinearInterpolator());
    renderInterpolator("Quad in", &Interpolator::TQuadInInterpolator());
    renderInterpolator("Quad out", &Interpolator::TQuadOutInterpolator());
    renderInterpolator("Quad in out", &Interpolator::TQuadInOutInterpolator());
    renderInterpolator("Cubic in", &Interpolator::TCubicInInterpolator());
    renderInterpolator("Cubic out", &Interpolator::TCubicOutInterpolator());
    renderInterpolator("Cubic in out", &Interpolator::TCubicInOutInterpolator());
    renderInterpolator("Quart in", &Interpolator::TQuartInInterpolator());
    renderInterpolator("Quart out", &Interpolator::TQuartOutInterpolator());
    renderInterpolator("Quart in out", &Interpolator::TQuartInOutInterpolator());
    renderInterpolator("Quint in", &Interpolator::TQuintInInterpolator());
    renderInterpolator("Quint out", &Interpolator::TQuintOutInterpolator());
    renderInterpolator("Quint in out", &Interpolator::TQuintInOutInterpolator());
    renderInterpolator("Back in", &Interpolator::TBackInInterpolator());
    renderInterpolator("Back out", &Interpolator::TBackOutInterpolator());
    renderInterpolator("Back in out", &Interpolator::TBackInOutInterpolator());
    renderInterpolator("Elastic in", &Interpolator::TElasticInInterpolator());
    renderInterpolator("Elastic out", &Interpolator::TElasticOutInterpolator());
    renderInterpolator("Elastic in out", &Interpolator::TElasticInOutInterpolator());
    renderInterpolator("Bounce in", &Interpolator::TBounceInInterpolator());
    renderInterpolator("Bounce out", &Interpolator::TBounceOutInterpolator());
    renderInterpolator("Bounce in out", &Interpolator::TBounceInOutInterpolator());
    renderInterpolator("Circular in", &Interpolator::TCircularInInterpolator());
    renderInterpolator("Circular out", &Interpolator::TCircularOutInterpolator());
    renderInterpolator("Circular in out", &Interpolator::TCircularInOutInterpolator());
    renderInterpolator("Expo in", &Interpolator::TExpoInInterpolator());
    renderInterpolator("Expo out", &Interpolator::TExpoOutInterpolator());
    renderInterpolator("Expo in out", &Interpolator::TExpoInOutInterpolator());
    renderInterpolator("Sine in", &Interpolator::TSineInInterpolator());
    renderInterpolator("Sine out", &Interpolator::TSineOutInterpolator());
    renderInterpolator("Sine in out", &Interpolator::TSineInOutInterpolator());

    ImGui::TreePop();
  }
}
