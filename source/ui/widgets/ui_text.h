#pragma once

#include "ui/ui_widget.h"

namespace UI
{
  class CText : public CWidget
  {
  public:
    void render() override;

  private:
    TTextParams _textParams;

    friend class CParser;
  };
}
