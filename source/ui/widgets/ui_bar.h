#pragma once

#include "ui/ui_widget.h"

namespace UI
{
  class CBar : public CWidget
  {
  public:
    void render() override;
    void setRatio(float newRatio);

  private:
    TImageParams _imageParams;
    TBarParams _barParams;

    friend class CParser;
  };
}
