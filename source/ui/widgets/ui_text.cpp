#pragma once

#include "mcv_platform.h"
#include "ui/widgets/ui_text.h"
#include "ui/ui_utils.h"

namespace UI
{
  void CText::render()
  {
    MAT44 world = _pivot * MAT44::CreateScale(_textParams.size.x, _textParams.size.y, 1.f) * _absolute;
    renderText(world, _textParams.texture, _textParams.text, _textParams.size);
  }
}
