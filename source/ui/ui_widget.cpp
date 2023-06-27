#pragma once

#include "mcv_platform.h"
#include "engine.h"
#include "ui/ui_widget.h"
#include "ui/ui_effect.h"
#include "windows/app.h"
#include "ui/module_ui.h"


namespace UI
{
  void CWidget::start()
  {
    for (auto fx : _effects)
    {
      fx->start();
    }

    for (auto& child : _children)
    {
      child->start();
    }
  }

  void CWidget::stop()
  {
    for (auto fx : _effects)
    {
      fx->stop();
    }

    for (auto& child : _children)
    {
      child->stop();
    }
  }

  void CWidget::update(float dt)
  {
    for (auto fx : _effects)
    {
      fx->update(dt);
    }

    for (auto& child : _children)
    {
      child->update(dt);
    }
  }

  void CWidget::doRender()
  {
    if (!_params.visible)
    {
      return;
    }

    render();

    for (auto& child : _children)
    {
      child->doRender();
    }
  }

  void CWidget::updateTransform()
  {
    computeAbsolute();

    for (auto& child : _children)
    {
      child->updateTransform();
    }
  }

  void CWidget::computePivot()
  {
    _pivot = MAT44::Identity * MAT44::CreateTranslation(-_params.pivot.x, -_params.pivot.y, 0.f);
  }

  void CWidget::computeLocal()
  {
    computePivot();

    MAT44 tr = MAT44::CreateTranslation(_params.position.x, _params.position.y, 0.f);
    MAT44 sc = MAT44::CreateScale(_params.scale.x, _params.scale.y, 0.f);
    MAT44 rot = MAT44::CreateRotationZ(_params.rotation);
    
    _local = rot * sc * tr;
  }

  void CWidget::computeAbsolute()
  {
    computeLocal();

    _absolute = _parent ? _local * _parent->_absolute : _local;
  }

  void CWidget::setParent(CWidget* parent)
  {
    removeFromParent();

    if (!parent)
    {
      return;
    }

    _parent = parent;
    _parent->_children.push_back(this);
  }

  void CWidget::removeFromParent()
  {
    if (!_parent)
    {
      return;
    }

    auto it = std::find(_parent->_children.begin(), _parent->_children.end(), this);
    assert(it != _parent->_children.end());
    _parent->_children.erase(it);
    _parent = nullptr;
  }

  void CWidget::childAppears(bool getFromChildren,bool alfaPos,float initial_time, float lerp_time) {

	  if (this == nullptr) {
		  return;
	  }
	  std::vector<CWidget*> _childrens;
	  if (getFromChildren) _childrens = _children[0]->_children;
	  else _childrens = _children;

	  for (int i = 0; i < _childrens.size(); i++) {
		  TImageParams* img = _childrens[i]->getImageParams();
		  if (img != nullptr) {
			  if (alfaPos) {
				  img->color.w = 0.0f;
				  CEngine::get().getUI().lerp(&img->color.w, 1.0f, initial_time, lerp_time);
				  //img->color.w = 1.0f;
			  }
			  else {
				  img->color.w = 1.0f;
				  CEngine::get().getUI().lerp(&img->color.w, 0.0f, initial_time, lerp_time);
			  }
		  }
	  }
  }




}
