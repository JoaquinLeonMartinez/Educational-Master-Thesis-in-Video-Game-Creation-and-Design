#pragma once

#include "ui/ui_widget.h"

namespace UI
{
  class CSprite : public CWidget
  {
  public:
    void render() override;
	void update(float dt) override;
	void initializeSprite();

  private:
    TImageParams _imageParams;
	TSpriteParams _spriteParams;
	float _time_since_start = 1.f;
	int _actual_horizontal_frame = 0;
	int _actual_vertical_frame = 0;
	int _actual_frame = 1;
    friend class CParser;
  };
}
