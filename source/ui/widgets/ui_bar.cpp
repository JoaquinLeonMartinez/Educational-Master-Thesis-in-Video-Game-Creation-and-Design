#pragma once

#include "mcv_platform.h"
#include "ui/widgets/ui_bar.h"
#include "ui/ui_utils.h"
#include "render/textures/texture.h"

namespace UI
{
  void CBar::render()
  {
    const MAT44 imageWorld = _pivot * MAT44::CreateScale(_imageParams.size.x * _barParams.ratio, _imageParams.size.y, 1.f) * _absolute;
    const VEC2 maxUV(_barParams.ratio, 1.f);
    renderBitmap(imageWorld, _imageParams.texture, VEC2::Zero, maxUV, _imageParams.color, _imageParams.additive);
  }

  void CBar::setRatio(float newRatio)
  {
	  _barParams.ratio = clamp(newRatio, 0.f, 1.f);
  }
}
