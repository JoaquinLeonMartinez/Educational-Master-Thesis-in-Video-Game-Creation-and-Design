#pragma once

#include "mcv_platform.h"
#include "ui/ui_parser.h"
#include "engine.h"
#include "ui/module_ui.h"
#include "ui/ui_widget.h"
#include "ui/widgets/ui_image.h"
#include "ui/widgets/ui_text.h"
#include "ui/widgets/ui_button.h"
#include "ui/widgets/ui_progress.h"
#include "ui/widgets/ui_bar.h"
#include "render/textures/texture.h"
#include "ui/effects/ui_fx_animate_uv.h"
#include "ui/effects/ui_fx_scale.h"

namespace UI
{
  void CParser::loadFile(const std::string& widgetsListFile)
  {
    json jData = loadJson(widgetsListFile);
    for (const json& jDataFile : jData)
    {
      loadWidget(jDataFile);
    }
  }

  void CParser::loadWidget(const std::string& widgetFile)
  {
    json jData = loadJson(widgetFile);
    
    CWidget* widget = parseWidget(jData, nullptr);

    widget->updateTransform();

    Engine.getUI().registerWidget(widget);
  }

  std::string CParser::loadFileByName(const std::string& file) {
	  json jData = loadJson(file);
	  std::string mainName;
	  CWidget* widget = parseWidget(jData, nullptr);
	  widget->updateTransform();
	 // widget->computeAbsolute();
	  Engine.getUI().registerWidget(widget);
	  const std::string& name = jData["name"];
	  return name;
  }
	



  CWidget* CParser::parseWidget(const json& jData, CWidget* parent)
  {
    const std::string& name = jData["name"];
    const std::string alias = jData.value<std::string>("alias", "");
    const std::string type = jData.value<std::string>("type", "widget");

    CWidget* widget = nullptr;

    if (type == "image")          widget = parseImage(jData);
    else if (type == "text")      widget = parseText(jData);
    else if (type == "button")    widget = parseButton(jData);
    else if (type == "progress")  widget = parseProgress(jData);
	else if (type == "bar")		  widget = parseBar(jData);
    else                          widget = parseWidget(jData);

    widget->_name = name;
    widget->_alias = alias;
    widget->setParent(parent);

    // parse effects
    if (jData.count("effects") > 0)
    {
      for (auto& jEffectData : jData["effects"])
      {
        CEffect* fx = parseEffect(jEffectData);
        if (fx)
        {
          fx->_owner = widget;
          widget->_effects.push_back(fx);
        }
      }
    }

    // parse children widgets
    if (jData.count("children") > 0)
    {
      for (auto& jChildrenData : jData["children"])
      {
        parseWidget(jChildrenData, widget);

      }
    }

    if (!alias.empty())
    {
      Engine.getUI().registerAlias(widget);
    }

    return widget;
  }
  
  CWidget* CParser::parseWidget(const json& jData)
  {
    CWidget* widget = new CWidget;

    parseParams(widget->_params, jData);

    return widget;
  }

  CWidget* CParser::parseImage(const json& jData)
  {
    CImage* image = new CImage;

    parseParams(image->_params, jData);
    parseParams(image->_imageParams, jData);

    return image;
  }

  CWidget* CParser::parseText(const json& jData)
  {
    CText* text = new CText;

    parseParams(text->_params, jData);
    parseParams(text->_textParams, jData);

    return text;
  }

  CWidget* CParser::parseButton(const json& jData)
  {
    CButton* button = new CButton;

    parseParams(button->_params, jData);

    CButton::TState defaultState;
    parseParams(defaultState.imageParams, jData);
    parseParams(defaultState.textParams, jData);

    button->_states["default"] = defaultState;

    assert(jData.count("states") > 0);
    for (auto& jState : jData["states"])
    {
      CButton::TState st = defaultState;
      
      parseParams(st.imageParams, jState);
      parseParams(st.textParams, jState);

      const std::string& stateName = jState["name"];

      button->_states[stateName] = st;
    }

    button->setCurrentState(button->_states.begin()->first);

    return button;
  }

  CWidget* CParser::parseProgress(const json& jData)
  {
    CProgress* progress = new CProgress;

    parseParams(progress->_params, jData);
    parseParams(progress->_imageParams, jData);
    parseParams(progress->_progressParams, jData);

    return progress;
  }

  CWidget* CParser::parseBar(const json& jData)
  {
	  CBar* bar = new CBar;

	  parseParams(bar->_params, jData);
	  parseParams(bar->_imageParams, jData);
	  parseParams(bar->_barParams, jData);

	  return bar;
  }

  void CParser::parseParams(TParams& params, const json& jData)
  {
    params.pivot = loadVEC2(jData, "pivot", params.pivot);
    params.position = loadVEC2(jData, "position", params.position);
    params.scale = loadVEC2(jData, "scale", params.scale);
    params.rotation = deg2rad(jData.value<float>("rotation", rad2deg(params.rotation)));
    params.visible = jData.value<bool>("visible", params.visible);
  }

  void CParser::parseParams(TImageParams& params, const json& jData)
  {
    params.size = loadVEC2(jData, "size", params.size);
    params.texture = jData.count("texture") > 0 ? Resources.get(jData["texture"])->as<CTexture>() : params.texture;
    params.additive = jData.value<bool>("additive", params.additive);
    params.color = loadColor(jData, "color", params.color * 255) / 255;
    params.minUV = loadVEC2(jData, "minUV", params.minUV);
    params.maxUV = loadVEC2(jData, "maxUV", params.maxUV);
  }

  void CParser::parseParams(TTextParams& params, const json& jData)
  {
    params.text = jData.value<std::string>("text", params.text);
    params.texture = jData.count("font_texture") > 0 ? Resources.get(jData["font_texture"])->as<CTexture>() : params.texture;
    params.size = loadVEC2(jData, "font_size", params.size);
  }

  void CParser::parseParams(TProgressParams& params, const json& jData)
  {
    params.ratio = jData.value<float>("text", params.ratio);
  }
  //de momento un ratio
  void CParser::parseParams(TBarParams& params, const json& jData)
  {
	  params.ratio = jData.value<float>("text", params.ratio);
  }



  CEffect* CParser::parseEffect(const json& jData)
  {
    const std::string type = jData.value<std::string>("type", "");

    CEffect* fx = nullptr;

    if (type == "animate_uv")      fx = parseFXAnimateUV(jData);
    else if (type == "scale")      fx = parseFXScale(jData);
    
    assert(fx);

    return fx;
  }

  CEffect* CParser::parseFXAnimateUV(const json& jData)
  {
    CFXAnimateUV* fx = new CFXAnimateUV;

    fx->_speed = loadVEC2(jData, "speed", fx->_speed);
	fx->name = jData.value<std::string>("name","");
    return fx;
  }

  CEffect* CParser::parseFXScale(const json& jData)
  {
    CFXScale* fx = new CFXScale;

    fx->_scale = loadVEC2(jData, "scale", fx->_scale);
    fx->_duration = jData.value<float>("duration", fx->_duration);
	fx->name = jData.value<std::string>("name", "");

    const std::string mode = jData.value<std::string>("mode", "single");
    if (mode == "single")         fx->_mode = CFXScale::EMode::Single;
    else if (mode == "loop")      fx->_mode = CFXScale::EMode::Loop;
    else if(mode == "ping_pong")  fx->_mode = CFXScale::EMode::PingPong;

    fx->_interpolator = parseInterpolator(jData.value<std::string>("interpolator", ""));

    return fx;
  }

  Interpolator::IInterpolator* CParser::parseInterpolator(const std::string& type)
  {
    static Interpolator::TLinearInterpolator linear;
    static Interpolator::TQuadInOutInterpolator quadInt;
    static Interpolator::TBackOutInterpolator backInt;
    static Interpolator::TBounceOutInterpolator bounceInt;

    if (type == "quad")         return &quadInt;
    else if (type == "back")    return &backInt;
    else if (type == "bounce")  return &bounceInt;
    else                        return &linear;
  }
}
