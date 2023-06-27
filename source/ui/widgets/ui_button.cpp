#pragma once

#include "mcv_platform.h"
#include "ui/widgets/ui_button.h"
#include "ui/ui_utils.h"
#include "render/textures/texture.h"

namespace UI
{
  void CButton::render()
  {
    if (!_currentState)
    {
      return;
    }

    MAT44 imageWorld = _pivot * MAT44::CreateScale(_currentState->imageParams.size.x, _currentState->imageParams.size.y, 1.f) * _absolute;
    renderBitmap(imageWorld, _currentState->imageParams.texture, VEC2::Zero, VEC2::One, _currentState->imageParams.color, _currentState->imageParams.additive);
    
    MAT44 textWorld = _pivot * MAT44::CreateScale(_currentState->textParams.size.x, _currentState->textParams.size.y, 1.f) * _absolute;
    renderText(textWorld, _currentState->textParams.texture, _currentState->textParams.text, _currentState->textParams.size);
  }

  void CButton::setCurrentState(const std::string& stateName)
  {
    auto it = _states.find(stateName);
    _currentState = it != _states.end() ? &it->second : nullptr;
  }
}
