#include "mcv_platform.h"
#include "module_game_intro.h"
#include "engine.h"
#include "input/input.h"
#include "components/common/comp_transform.h"
#include "ui/controllers/ui_menu_controller.h"
#include "ui/module_ui.h"
#include "ui/widgets/ui_button.h"
#include "ui/ui_widget.h"




bool CModuleGameIntro::start()
{
	UI::CModuleUI& ui = Engine.getUI();
	CEngine::get().getUI().activateWidgetClass("INTRO_SCREEN")->childAppears(true, true, 0.0, 1.25);
	auto& app = CApplication::get();
	if (app.cursorIngame) {
		Input::CMouse mouse = EngineInput.mouse();
		mouse.setLockMouse(true);
	}



	Scripting.execActionDelayed("stopWidgetEffectSpecial()", 0);
	Scripting.execActionDelayed("changeDurationWidgetEffectSpecial(1)", 1);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 4);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 7);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 10);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 13);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(-0.5, 0.20)", 15);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 16.25);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 18);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 21.2);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 24.2);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 27.2);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(-0.5, 0.20)", 29.2);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 30.40);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 33.0);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 36.0);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 39.0);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 42.60);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(-0.5, 0.18)", 44.60);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 45.90);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 48.0);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 51.0);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.1, 0.0)", 54.0);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(0.0, 0.0)", 57.5);
	Scripting.execActionDelayed("childAppears(\"LOAD_SCREEN\",true,true,0.0,1.0)", 60);
    Scripting.execActionDelayed("changeGameState(\"gs_loading\")", 61.5);
    Scripting.execActionDelayed("GameController:updateSoundtrackID(2)", 62);

    GameController.updateSoundtrackID(7);
	/*
	Scripting.execActionDelayed("stopWidgetEffectSpecial()", 2);
	Scripting.execActionDelayed("changeSpeedWidgetEffectSpecial(-0.85, 0.5)", 3);
	Scripting.execActionDelayed("stopWidgetEffectSpecial()",5);


	Scripting.execActionDelayed("childAppears(\"LOAD_SCREEN\",true,true,0.0,1.0)", 6);
	Scripting.execActionDelayed("changeGameState(\"gs_loading\")", 8);
	*/
	return true;
}

void CModuleGameIntro::update(float delta)
{
	
}

void CModuleGameIntro::stop()
{
	CEngine::get().getUI().deactivateWidgetClass("INTRO_SCREEN");
	CEngine::get().getUI().deactivateWidgetClass("BLACK_SCREEN");
}

void CModuleGameIntro::renderInMenu()
{

}

void CModuleGameIntro::renderDebug()
{

}
