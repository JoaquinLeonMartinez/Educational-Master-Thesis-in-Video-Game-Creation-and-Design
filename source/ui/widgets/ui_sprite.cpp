#pragma once

#include "mcv_platform.h"
#include "ui/widgets/ui_sprite.h"
#include "ui/ui_utils.h"
#include "render/textures/texture.h"
#include "ui/ui_utils.h"

namespace UI
{
	void CSprite::initializeSprite() {
		_imageParams.minUV = VEC2(0, 0);
		_imageParams.maxUV = _spriteParams._frame_size[_spriteParams._playing_sprite] / _spriteParams._original_image_size[_spriteParams._playing_sprite];

	}

	void CSprite::render() {

		MAT44 sz = MAT44::CreateScale(_imageParams.size.x, _imageParams.size.y, 1.f);

		renderBitmap(sz * _absolute,
			_spriteParams._textures[_spriteParams._playing_sprite],
			_imageParams.minUV,
			_imageParams.maxUV,
			_imageParams.color);
	}


	void CSprite::update(float dt) {

		_time_since_start += dt;

		if (_time_since_start >= (1.f / (float)_spriteParams._frames_per_second[_spriteParams._playing_sprite])) {

			_actual_horizontal_frame++;
			_actual_frame++;
			if (_actual_frame > _spriteParams.numFrames[_spriteParams._playing_sprite]) {
				_actual_frame = 1;
				_actual_horizontal_frame = 0;
				_actual_vertical_frame = 0;
			}

			if (_actual_horizontal_frame == (int)(_spriteParams._original_image_size[_spriteParams._playing_sprite].x / _spriteParams._frame_size[_spriteParams._playing_sprite].x)) {
				_actual_horizontal_frame = 0;
				_actual_vertical_frame++;
			}

			if (_actual_vertical_frame == (int)(_spriteParams._original_image_size[_spriteParams._playing_sprite].y / _spriteParams._frame_size[_spriteParams._playing_sprite].y)) {
				_actual_vertical_frame = 0;
				_actual_frame = 1;
			}

			VEC2 _aux_minUV;
			_aux_minUV.x = (_spriteParams._frame_size[_spriteParams._playing_sprite].x) / (_spriteParams._original_image_size[_spriteParams._playing_sprite].x) * _actual_horizontal_frame;
			_aux_minUV.y = (_spriteParams._frame_size[_spriteParams._playing_sprite].y) / (_spriteParams._original_image_size[_spriteParams._playing_sprite].y) * _actual_vertical_frame;

			VEC2 _aux_maxUV;
			_aux_maxUV.x = (_spriteParams._frame_size[_spriteParams._playing_sprite].x) / (_spriteParams._original_image_size[_spriteParams._playing_sprite].x) * (_actual_horizontal_frame + 1);
			_aux_maxUV.y = (_spriteParams._frame_size[_spriteParams._playing_sprite].y) / (_spriteParams._original_image_size[_spriteParams._playing_sprite].y) * (_actual_vertical_frame + 1);
			_imageParams.minUV = _aux_minUV;
			_imageParams.maxUV = _aux_maxUV;
			_time_since_start = 0.0f;
		}
	}

  
}
