#include "mcv_platform.h"
#include "module_game_ui.h"
#include "entity/entity.h"
#include "engine.h"
#include "input/input.h"
#include "components/common/comp_transform.h"
#include "ui/module_ui.h"
#include "ui/widgets/ui_bar.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/powers/comp_madness.h"
#include "ui/controllers/ui_menu_controller.h"
#include "ui/widgets/ui_button.h"
#include "ui/widgets/ui_image.h"
#include "components/powers/comp_teleport.h"
#include "components/powers/comp_coffee.h"
#include "components/powers/comp_fire.h"

bool CModuleGameUI::start()
{
	UI::CModuleUI& ui = Engine.getUI();
	//ui.activateWidget("game_ui");
	if(ui.sizeUI == 1){
		//CEngine::get().getUI().activateWidgetClass("HUD_NORMAL_PLAYER");
		Scripting.execActionDelayed("activateWidget(\"HUD_NORMAL_PLAYER\")", 2.32);
		
	}
	else {
		CEngine::get().getUI().activateWidgetClass("HUD_NORMAL_PLAYER_MINI");
	}
	/*
	NO CREAR UN CMENUCONTROLER EN EL MODULO GAMEPLAY--- SI SE NECESITA CREAR OTRO 
	UI::CMenuController* menu = new UI::CMenuController; 
	
	menu->registerOption(dynamic_cast<UI::CButton*>(ui.getWidgetByAlias("bt_carro_")), std::bind(&CModuleGameUI::onOptionCarrito, this));
	menu->registerOption(dynamic_cast<UI::CButton*>(ui.getWidgetByAlias("bt_dash_")), std::bind(&CModuleGameUI::onOptionDash, this));
	menu->registerOption(dynamic_cast<UI::CButton*>(ui.getWidgetByAlias("bt_jump_")), std::bind(&CModuleGameUI::onOptionJump, this));
	menu->registerOption(dynamic_cast<UI::CButton*>(ui.getWidgetByAlias("bt_mop_")), std::bind(&CModuleGameUI::onOptionMop, this));
	
	

	//ui.registerController(menu);
	*/


  return true;
}

void CModuleGameUI::update(float delta)
{
	PROFILE_FUNCTION("CModuleGameUI::update");
	CEntity* e_player = getEntityByName("Player");
	CEntity* e_inventory = getEntityByName("Inventory");
	/*if (EngineInput["pause"].justPressed()) {
		CEngine::get().getModules().changeToGamestate("gs_paused");//change gamestate
		//pause game
        GameController.pauseGame();
		//Time.real_scale_factor = 0.0f;
	}*/

	if(Time.real_scale_factor != 0.0f){
		if (EngineInput["jump_"].isPressed() && EngineInput["jump_"].timePressed < 0.5f) {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_jump_"));
			boton->setCurrentState("selected");
		}
		else {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_jump_"));
			boton->setCurrentState("enabled");
		}

		if (EngineInput["dash_"].isPressed() && EngineInput["dash_"].timePressed < 0.5f) {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_dash_"));
			boton->setCurrentState("selected");
		}
		else {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_dash_"));
			boton->setCurrentState("enabled");
		}

		if (EngineInput["interact_"].isPressed() && EngineInput["interact_"].timePressed < 0.5f) {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_carro_"));
			boton->setCurrentState("selected");
		}
		else {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_carro_"));
			boton->setCurrentState("enabled");
		}
		if (EngineInput["attack_"].isPressed() && EngineInput["attack_"].timePressed < 0.5f) {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_mop_"));
			boton->setCurrentState("selected");
		}
		else {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_mop_"));
			boton->setCurrentState("enabled");
		}
		TCompCharacterController* c_controller = e_player->get<TCompCharacterController>();
		TCompInventory* inventory = e_inventory->get<TCompInventory>();
		if (inventory->getBattery()) {
			if(!c_controller->cinematic) {
				if (EngineInput["select_battery_"].justPressed()) {
					UI::CButton* b = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("card_"));
					b->setCurrentState("option_battery");
					//cambiar imagen de cursor un instante de timepo
					UI::CButton* b_cursor = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("cursor_"));
					b_cursor->setCurrentState("option_battery");


				}
			}
		}

		if (inventory->getTeleport()){
			if (!c_controller->cinematic) {
				if (EngineInput["select_teleport_"].justPressed()) {
					UI::CButton* b = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("card_"));
					b->setCurrentState("option_teleport");
					//cambiar imagen de cursor un instante de timepo
					UI::CButton* b_cursor = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("cursor_"));
					b_cursor->setCurrentState("option_teleport");
				}
			}
		}
		if (inventory->getChilli()) {
			UI::CButton* b = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_r1_"));
			b->getParams()->visible = true;
			TCompFireController* fire = e_player->get<TCompFireController>();
			if (fire->isEnabled()) {
				b->setCurrentState("selected");
			}
			else {
				b->setCurrentState("enabled");
			}
		}else{
			UI::CButton* b = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_r1_"));
			b->getParams()->visible = false;
		}

		if (inventory->getCoffe()) {
			UI::CButton* b = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_l1_"));
			b->getParams()->visible = true;
			TCompCoffeeController* tc = e_player->get<TCompCoffeeController>();
			if (tc->getIsEnabled()) {
				b->setCurrentState("selected");
			}
			else {
				b->setCurrentState("enabled");
			}
		}
		else {
			UI::CButton* b = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_l1_"));
			b->getParams()->visible = false;
		}




		/*
		//ANTES PARA QLOS DISPAROS-- NO ERA CORRECTO PORQUE ERAN ITEMS(CAFE Y FUEGO)
		if (EngineInput["select_aim_"].isPressed()) {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_l1_"));
			boton->setCurrentState("selected");
		}
		else {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_l1_"));
			boton->setCurrentState("enabled");
		}

		
		TCompTeleport* c_tp = e_player->get<TCompTeleport>();
		if (c_tp->timeAfterTeleportUI <= 0) {
			UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("bt_r1_"));
			boton->setCurrentState("enabled");
			c_tp->timeAfterTeleportUI = 0.5f;
		}
		*/
		
	
		if (e_player != nullptr) {
			//Mana
			TCompCharacterController* c_controller = e_player->get<TCompCharacterController>();
			TCompMadnessController* madness_controller = e_player->get<TCompMadnessController>();
			UI::CBar* bar = dynamic_cast<UI::CBar*>(Engine.getUI().getWidgetByAlias("mana_bar_r"));
      UI::CBar* bar2 = dynamic_cast<UI::CBar*>(Engine.getUI().getWidgetByAlias("life_bar_r"));
      //float realMadnessBar = (madness_controller->getRemainingMadness() + 20) / (c_controller->getMaxMadness() + 20);//ofset de las barras de vida
	  float realMadnessBar = (madness_controller->getRemainingMadness()) / (c_controller->getMaxMadness() ) + 0.01;//ofset de las barras de locura
      //float realLifeBar = (c_controller->life + 20) / 120.f;
	  float realLifeBar = (c_controller->life + 0.01) / 100.f;

      if (actualLifeRatioBar == -1.0f) {
        actualLifeRatioBar = realLifeBar;
        bar2->setRatio(actualLifeRatioBar); // 20 y 120 son offset de la barra de vida
      }
        

      if (actualMadnessRatioBar == -1.0f) {
        actualMadnessRatioBar = realMadnessBar;
        bar->setRatio(actualMadnessRatioBar);
      }

      float dirLifeBar = realLifeBar - actualLifeRatioBar;
      float dirMadnessBar = realMadnessBar - actualMadnessRatioBar;

      if (abs(dirLifeBar) > 0.01f) {
        if (dirLifeBar > 0.0f) {
          actualLifeRatioBar = clamp(actualLifeRatioBar + (sign(dirLifeBar) * delta * 0.5f), 0.0f, realLifeBar);
        }
        else {
          actualLifeRatioBar = clamp(actualLifeRatioBar + (sign(dirLifeBar) * delta * 0.5f), realLifeBar, 1.0f);
        }
        bar2->setRatio(actualLifeRatioBar); // 20 y 120 son offset de la barra de vida
      }

      if (abs(dirMadnessBar) > 0.01f) {
        if (dirMadnessBar > 0.0f) {
          actualMadnessRatioBar = clamp(actualMadnessRatioBar + (sign(dirMadnessBar) * delta * 0.5f), 0.0f, realMadnessBar);
        }
        else {
          actualMadnessRatioBar = clamp(actualMadnessRatioBar + (sign(dirMadnessBar) * delta * 0.5f), realMadnessBar, 1.0f);
        }
        bar->setRatio(actualMadnessRatioBar);
      }

		}
	}
  
}

void CModuleGameUI::onOptionCarrito() {
	dbg("Opcion carrito \n");
}

void CModuleGameUI::onOptionDash() {
	dbg("Opcion dash \n");
}

void CModuleGameUI::onOptionJump() {
	dbg("Opcion jump \n");
}

void CModuleGameUI::onOptionMop() {
	dbg("Opcion mop \n");
}

void CModuleGameUI::stop()
{

}

void CModuleGameUI::renderInMenu()
{

}

void CModuleGameUI::renderDebug()
{

}