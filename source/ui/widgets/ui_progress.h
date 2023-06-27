#pragma once

#include "ui/ui_widget.h"

namespace UI
{
  class CProgress : public CWidget
  {
  public:
    void render() override;
    void setRatio(float newRatio);

  private:
    TImageParams _imageParams;
    TProgressParams _progressParams;

    friend class CParser;
  };
}
