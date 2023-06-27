#pragma once

#include "mcv_platform.h"
#include "ui/widgets/ui_progress.h"
#include "ui/ui_utils.h"
#include "render/textures/texture.h"

namespace UI
{
  void CProgress::render()
  {
    const MAT44 imageWorld = _pivot * MAT44::CreateScale(_imageParams.size.x * _progressParams.ratio, _imageParams.size.y, 1.f) * _absolute;
    const VEC2 maxUV(_progressParams.ratio, 1.f);
    renderBitmap(imageWorld, _imageParams.texture, VEC2::Zero, maxUV, _imageParams.color, _imageParams.additive);
  }

  void CProgress::setRatio(float newRatio)
  {
    _progressParams.ratio = clamp(newRatio, 0.f, 1.f);
  }
}
