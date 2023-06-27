#pragma once

#include "modules/module.h"

namespace UI
{
  class CWidget;
  class CController;

  class CModuleUI : public IModule
  {
  public:
    CModuleUI(const std::string& name);

	struct WidgetClass {
		std::string name;//nombre del widget en el json es el name
		std::string type; //Tipo de si es vida, stamina, boton, creditos, fondo negro... (lo que se nos antoje)
		CWidget *widget;
		CController *_controller;
		bool enabled = false;
	};

	std::map<std::string, WidgetClass> _widgetStructureMap;
    bool start() override;
    void update(float dt) override;
    void render();

    void renderInMenu() override;
	void unregisterController();
	void unregisterController(CController* controller);
    void registerWidget(CWidget* widget);
    void activateWidget(const std::string& name);
    void deactivateWidget(const std::string& name);
	//Nuevos metodos pra la causa
	void registerWidgetClass(std::string wdgt_type, std::string wdgt_path, CController *wdgt_controller = nullptr);
	void initWidgetClass();
	CWidget* activateWidgetClass(const std::string& name);
	void deactivateWidgetClass(const std::string& name);
	void stopWidgetEffect(const std::string& nameWidgetStrMap, const std::string& nameEffect);
	void stopWidgetEffect(UI::CWidget& widget, const std::string& nameEffect);
	void changeSpeedWidgetEffect(const std::string& nameWidgetStrMap, const std::string& nameEffect, float x, float y);
	CWidget* getWidget(const std::string& nameWidgetStrMap);


	CController* getWidgetController(std::string type);
    void registerAlias(CWidget* widget);
    void registerController(CController* controller);

    CWidget* getWidgetByName(const std::string& name);
    CWidget* getWidgetByAlias(const std::string& alias);



	void lerp(float *init_value, float value_to_lerp, float initial_time, float lerp_time);


	struct WidgetToLerp {

		float *element;
		float maxElement;
		float value;
		float initialTime = 0.0;
		float lerpTime;
		float currentTime = 0.0;
		bool isFirstFrame = true;
	};

	std::vector<WidgetToLerp>widgetsToLerp;
	int sizeUI; //0 es UI JUEGO 1 es UI pequeña 
	int botonPulsadoGameOver;//0 restart, 1 quit
	int botonPulsadoPause;//0 restart, 1 quit

  private:
    std::map<std::string_view, CWidget*> _registeredWidgets;
    std::map<std::string_view, CWidget*> _registeredAlias;
    std::vector<CWidget*> _activeWidgets;
    std::vector<CController*> _activeControllers;
  };
}
