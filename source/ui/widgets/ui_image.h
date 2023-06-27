#pragma once

#include "ui/ui_widget.h"

namespace UI
{
  class CImage : public CWidget
  {
  public:
    void render() override;
    TImageParams* getImageParams() override { return &_imageParams; }
	

  private:
    TImageParams _imageParams;

    friend class CParser;
  };
}
