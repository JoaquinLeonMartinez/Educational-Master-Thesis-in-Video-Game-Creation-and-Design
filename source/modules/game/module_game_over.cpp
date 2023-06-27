#include "mcv_platform.h"
#include "module_game_over.h"
#include "engine.h"
#include "input/input.h"
#include "modules/module_boot.h"
#include "components/common/comp_transform.h"
#include "ui/controllers/ui_menu_controller.h"
#include "ui/module_ui.h"
#include "ui/widgets/ui_button.h"
#include "components/postfx/comp_chromatic_aberration.h"

bool CModuleGameOver::start()
{
	UI::CModuleUI& ui = Engine.getUI();
	if (ui.sizeUI == 1) {
		CEngine::get().getUI().activateWidgetClass("BLACK_SCREEN")->childAppears(true, true, 0.0, 2.0);
		CEngine::get().getUI().activateWidgetClass("DEAD_MENU_BACKGROUND")->childAppears(true, true, 0.0, 2.0);
		CEngine::get().getUI().activateWidgetClass("DEAD_MENU_BUTTONS");
		CEngine::get().getUI().deactivateWidgetClass("HUD_NORMAL_PLAYER");
	}
	else {
		CEngine::get().getUI().activateWidgetClass("BLACK_SCREEN")->childAppears(true, true, 0.0, 1.0);
		CEngine::get().getUI().activateWidgetClass("DEAD_MENU_BACKGROUND_MINI")->childAppears(true, true, 0.0, 1.0);
		CEngine::get().getUI().activateWidgetClass("DEAD_MENU_BUTTONS_MINI");
		CEngine::get().getUI().deactivateWidgetClass("HUD_NORMAL_PLAYER_MINI");
	}
	//guardar el poder del characterController
	CEntity* e_player = GameController.getPlayerHandle();
	TCompCharacterController* characterController = e_player->get<TCompCharacterController>();
	PowerType power = characterController->power_selected;
	GameController.savePower(power);
	CEntity* m_camera = getEntityByName("MainCamera");
	TCompChromaticAberration* render_chromatic = m_camera->get<TCompChromaticAberration>();
	render_chromatic->update_amount = false;
	render_chromatic->amount = 0.2f;
	GameController.setResurrect(true);
  
	return true;
}

void CModuleGameOver::update(float delta)
{
	
}

void CModuleGameOver::stop()
{
	
	UI::CModuleUI& ui = Engine.getUI();
	if (ui.sizeUI == 1) {
		if(ui.botonPulsadoGameOver == 0) {//hemos pulsado restart
			GameController.resetCamera();
		}

		/*
		UI::CButton* b = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("card_"));
		b->setCurrentState("option_teleport");
		UI::CButton* b_cursor = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("cursor_"));
		b_cursor->setCurrentState("option_teleport");
		*/
		

		Scripting.execActionDelayed("deactivateWidget(\"BLACK_SCREEN\")", 0.0);
		Scripting.execActionDelayed("deactivateWidget(\"DEAD_MENU_BACKGROUND\")", 0.0);
		Scripting.execActionDelayed("deactivateWidget(\"DEAD_MENU_BUTTONS\")", 0.0);
		Scripting.execActionDelayed("activateWidget(\"HUD_NORMAL_PLAYER\")", 0.0);
		//CEngine::get().getUI().deactivateWidgetClass("BLACK_SCREEN");
		//CEngine::get().getUI().deactivateWidgetClass("DEAD_MENU_BACKGROUND");
		//CEngine::get().getUI().deactivateWidgetClass("DEAD_MENU_BUTTONS");
		//CEngine::get().getUI().activateWidgetClass("HUD_NORMAL_PLAYER");
		
	}
	else {
		//GameController.resetCamera();
		CEngine::get().getUI().deactivateWidgetClass("BLACK_SCREEN");
		CEngine::get().getUI().deactivateWidgetClass("DEAD_MENU_BACKGROUND_MINI");
		CEngine::get().getUI().deactivateWidgetClass("DEAD_MENU_BUTTONS_MINI");
		CEngine::get().getUI().activateWidgetClass("HUD_NORMAL_PLAYER");
	}

  CEntity* m_camera = getEntityByName("MainCamera");
  TCompChromaticAberration* render_chromatic = m_camera->get<TCompChromaticAberration>();
  render_chromatic->update_amount = true;
  //
  Scripting.execActionDelayed("setResurrect(false)", 1.0);
  
  
}

void CModuleGameOver::renderInMenu()
{

}

void CModuleGameOver::renderDebug()
{

}
