#pragma once

namespace UI
{
  class CWidget;
  class CEffect;
  struct TParams;
  struct TImageParams;
  struct TTextParams;
  struct TProgressParams;
  struct TBarParams;


  class CParser
  {
  public:
    void loadFile(const std::string& widgetsListFile);
    void loadWidget(const std::string& widgetFile);
	std::string loadFileByName(const std::string& file);


    CWidget* parseWidget(const json& jData, CWidget* parent);

    CWidget* parseWidget(const json& jData);
    CWidget* parseImage(const json& jData);
    CWidget* parseText(const json& jData);
    CWidget* parseButton(const json& jData);
    CWidget* parseProgress(const json& jData);
	CWidget* parseBar(const json& jData);

    void parseParams(TParams& params, const json& jData);
    void parseParams(TImageParams& params, const json& jData);
    void parseParams(TTextParams& params, const json& jData);
    void parseParams(TProgressParams& params, const json& jData);
	void parseParams(TBarParams& params, const json& jData);

    CEffect* parseEffect(const json& jData);
    CEffect* parseFXAnimateUV(const json& jData);
    CEffect* parseFXScale(const json& jData);

    Interpolator::IInterpolator* parseInterpolator(const std::string& type);
  };
}
