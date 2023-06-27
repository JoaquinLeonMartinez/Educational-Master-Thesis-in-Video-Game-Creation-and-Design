#include "mcv_platform.h"
#include "module_win_game.h"
#include "engine.h"
#include "render/render.h"
#include "input/input.h"
#include "modules/module_boot.h"
#include "components/common/comp_transform.h"
#include "ui/controllers/ui_menu_controller.h"
#include "ui/module_ui.h"
#include "ui/widgets/ui_button.h"
#include "module_game_controller.h"
#include "render/module_render.h"

bool CModuleWinGame::start()
{

	auto& app = CApplication::get();
	if (app.cursorIngame) {
		Input::CMouse mouse = EngineInput.mouse();
		mouse.setLockMouse(true);
	}
	UI::CModuleUI& ui = Engine.getUI();
	if (ui.sizeUI == 1) {
		CEngine::get().getUI().activateWidgetClass("BLACK_SCREEN")->childAppears(true, true, 0.0, 2.0);
		Scripting.execActionDelayed("activateWidget(\"CREDITS\")",0);
		Scripting.execActionDelayed("changeSpeedWidgetEffect(\"CREDITS\",\"effectAnimateCredit\",0,0.012)", 1);
		Scripting.execActionDelayed("stopWidgetEffect(\"CREDITS\",\"effectAnimateCredit\")", 72);
		Scripting.execActionDelayed("exitGame()",82);

	}
	else {
		CEngine::get().getUI().activateWidgetClass("BLACK_SCREEN")->childAppears(true, true, 0.0, 2.0);
	}
	
	
    return true;
}

void CModuleWinGame::update(float delta)
{
	/*timeOffModuleGameWin--;
	if (timeOffModuleGameWin <= 0.f) {
		//GameController.changeGameState("gs_main_menu");
		auto& app = CApplication::get();
		DestroyWindow(app.getHandle());
	}*/
	//GameController.changeGameState("gs_main_menu");
	
}

void CModuleWinGame::stop()
{
	
	UI::CModuleUI& ui = Engine.getUI();
	if (ui.sizeUI == 1) {
		CEngine::get().getUI().deactivateWidgetClass("BLACK_SCREEN");
	}
	else {
		CEngine::get().getUI().deactivateWidgetClass("BLACK_SCREEN");
	}

	/*
	EngineNavmesh.destroyNavmesh();
	EngineEntities.stop();
	Engine.getGPUCulling().stop();
	//EnginePhysics.stop();
	Resources.deleteResources();
	//Render.destroy();
	auto& mod_render = CEngine::get().getRender();
	
	mod_render.start();
	//EnginePhysics.start();

	//CEngine::get().getUI().stop();
	//CEngine::get().start();
	//EngineEntities.start();
	*/
}

void CModuleWinGame::renderInMenu()
{

}

void CModuleWinGame::renderDebug()
{

}
