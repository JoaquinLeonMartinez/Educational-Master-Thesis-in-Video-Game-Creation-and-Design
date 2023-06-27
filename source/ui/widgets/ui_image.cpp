#pragma once

#include "mcv_platform.h"
#include "ui/widgets/ui_image.h"
#include "ui/ui_utils.h"

namespace UI
{
  void CImage::render()
  {
    // render screen quad with texture
    MAT44 world = _pivot * MAT44::CreateScale(_imageParams.size.x, _imageParams.size.y, 1.f) * _absolute;
    renderBitmap(world, _imageParams.texture, _imageParams.minUV, _imageParams.maxUV, _imageParams.color, _imageParams.additive);
  }
}
