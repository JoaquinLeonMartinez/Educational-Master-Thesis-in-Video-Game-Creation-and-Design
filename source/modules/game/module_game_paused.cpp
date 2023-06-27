#include "mcv_platform.h"
#include "module_game_paused.h"
#include "entity/entity.h"
#include "engine.h"
#include "input/input.h"
#include "components/common/comp_transform.h"
#include "ui/controllers/ui_menu_controller.h"
#include "ui/module_ui.h"
#include "ui/widgets/ui_button.h"
#include "components/postfx/comp_render_blur.h"
#include "components/postfx/comp_chromatic_aberration.h"
bool CModuleGamePaused::start()
{


	UI::CModuleUI& ui = Engine.getUI();

	if (ui.sizeUI == 1) {
		CEngine::get().getUI().activateWidgetClass("PAUSE_MENU_BACKGROUND");
		CEngine::get().getUI().activateWidgetClass("PAUSE_MENU_BUTTONS");
		CEngine::get().getUI().deactivateWidgetClass("HUD_NORMAL_PLAYER");
		
	}
	else {
		CEngine::get().getUI().activateWidgetClass("PAUSE_MENU_BACKGROUND_MINI");
		CEngine::get().getUI().activateWidgetClass("PAUSE_MENU_BUTTONS_MINI");
		CEngine::get().getUI().deactivateWidgetClass("HUD_NORMAL_PLAYER_MINI");
		
	}
	CEntity* m_camera = getEntityByName("MainCamera");
	TCompRenderBlur* c_rrb = m_camera->get<TCompRenderBlur>();
  c_rrb->enabled = true;
	
	TCompChromaticAberration* render_chromatic = m_camera->get<TCompChromaticAberration>();
	render_chromatic->enabled = false;
	

	
	/*
	UI::CModuleUI& ui = Engine.getUI();
	ui.activateWidget("pausa_menu");

	UI::CMenuController* menu = new UI::CMenuController;
	
	menu->registerOption(dynamic_cast<UI::CButton*>(ui.getWidgetByAlias("bt_continue_")), std::bind(&CModuleGamePaused::onOptionContinue, this));
	menu->registerOption(dynamic_cast<UI::CButton*>(ui.getWidgetByAlias("bt_restart_")), std::bind(&CModuleGamePaused::onOptionRestart, this));
	menu->registerOption(dynamic_cast<UI::CButton*>(ui.getWidgetByAlias("bt_exit_")), std::bind(&CModuleGamePaused::onOptionExit, this));

	menu->setCurrentOption(0);

	ui.registerController(menu);
	*/


  return true;
}

void CModuleGamePaused::update(float delta)
{
  PROFILE_FUNCTION("CModuleGamePaused::update");
  if (EngineInput["pause"].justPressed()) {
    CEngine::get().getModules().changeToGamestate("gs_gameplay");//change gamestate
    //unpause game

    GameController.resumeGame();
    Time.real_scale_factor = 1.0f;
	UI::CModuleUI& ui = Engine.getUI();
	ui.botonPulsadoPause = 1;

  }
  
}

void CModuleGamePaused::stop()
{
	UI::CModuleUI& ui = Engine.getUI();
	if (ui.sizeUI == 1) {
		if(ui.botonPulsadoPause == 0) {
			Scripting.execActionDelayed("deactivateWidget(\"BLACK_SCREEN\")", 1.5);
			Scripting.execActionDelayed("deactivateWidget(\"PAUSE_MENU_BACKGROUND\")", 1.5);
			Scripting.execActionDelayed("deactivateWidget(\"PAUSE_MENU_BUTTONS\")", 1.5);
			Scripting.execActionDelayed("activateWidget(\"HUD_NORMAL_PLAYER\")", 1.5);
		}
		else {
			CEngine::get().getUI().deactivateWidgetClass("PAUSE_MENU_BACKGROUND");
			CEngine::get().getUI().deactivateWidgetClass("PAUSE_MENU_BUTTONS");
			CEngine::get().getUI().activateWidgetClass("HUD_NORMAL_PLAYER");
		}
	}
	else {
		CEngine::get().getUI().deactivateWidgetClass("PAUSE_MENU_BACKGROUND_MINI");
		CEngine::get().getUI().deactivateWidgetClass("PAUSE_MENU_BUTTONS_MINI");
		CEngine::get().getUI().activateWidgetClass("HUD_NORMAL_PLAYER_MINI");
	}
	CEntity* m_camera = getEntityByName("MainCamera");
  TCompRenderBlur* c_rrb = m_camera->get<TCompRenderBlur>();
  c_rrb->enabled = false;
	TCompChromaticAberration* render_chromatic = m_camera->get<TCompChromaticAberration>();
	render_chromatic->enabled = true;
	CEntity* e_player = GameController.getPlayerHandle();
	TCompCharacterController* characterController = e_player->get<TCompCharacterController>();
	Scripting.execActionDelayed("pausedPlayer(false)", 1.0);
	Scripting.execActionDelayed("setResurrect(false)", 1.0);
}

void CModuleGamePaused::renderInMenu()
{

}

void CModuleGamePaused::renderDebug()
{

}
/*
void CModuleGamePaused::onOptionContinue() {
	
	Time.real_scale_factor = 1.0f;
	CEngine::get().getModules().changeToGamestate("gs_gameplay");//change gamestate
	UI::CModuleUI& ui = Engine.getUI();
	ui.unregisterController();
    GameController.resumeGame();
}

void CModuleGamePaused::onOptionRestart() {
	
	CEngine::get().getModules().changeToGamestate("gs_gameplay");//change gamestate
	Time.real_scale_factor = 1.0f;
	UI::CModuleUI& ui = Engine.getUI();
	ui.unregisterController();
	GameController.loadCheckpoint();

}

void CModuleGamePaused::onOptionExit() {
	auto& app = CApplication::get();
	DestroyWindow(app.getHandle());
	
}*/