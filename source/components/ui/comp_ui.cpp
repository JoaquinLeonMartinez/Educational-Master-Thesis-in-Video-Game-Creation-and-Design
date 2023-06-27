#include "mcv_platform.h"
#include "comp_ui.h"
#include "components/common/comp_base.h"
#include "entity/entity_parser.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/powers/comp_madness.h"
#include "components/powers/comp_teleport.h"
#include "input/module_input.h"
#include "windows/app.h"
#include "engine.h"

DECL_OBJ_MANAGER("ui_controller", TCompUi);

void TCompUi::debugInMenu() {
	ImGui::DragFloat("Window Size", &windowSize, 10.0f, 0.0f, 1000.0f);
	ImGui::DragFloat("Reticle Thickness", &thick, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat4("Color", &col.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Exterior reticle", &sizeExteriorCross, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("Interior reticle", &sizeInteriorCross, 0.1f, 0.0f, 100.0f);
}

void TCompUi::load(const json& j, TEntityParseContext& ctx) {

}


void TCompUi::renderDebug() {
	ImVec2 _reticleWindowSize = ImVec2(windowSize, windowSize);

	ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
	ImGui::GetStyle().WindowRounding = 0;
	ImGui::GetStyle().WindowBorderSize = 1;

	float posX = (Render.width / 2 - _reticleWindowSize.x);
	float posY = (Render.width / 2 - _reticleWindowSize.y);
}

void TCompUi::update(float delta) {
	PROFILE_FUNCTION("ComponentUI");
	/*
		bool p_open = NULL;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs;


		ImVec2 _reticleWindowSize = ImVec2(windowSize, windowSize);
		ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
		ImGui::SetNextWindowSize(_reticleWindowSize, ImGuiCond_Always);

		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground
				| ImGuiWindowFlags_NoBringToFrontOnFocus;
			// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only do it to make the Demo applications a little more welcoming.
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_Always);

			// Main body of the Demo window starts here.
			ImGui::Begin("ImGui Demo", NULL, window_flags);

			CHandle h_player = getEntityByName("Player");
			CEntity* e_player = h_player;
			if (e_player != nullptr) {
				//Life
				TCompCharacterController* c_controller = e_player->get<TCompCharacterController>();

				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); //color
				ImGui::ProgressBar(c_controller->life / c_controller->maxLife, ImVec2(GameController.getHpBarSize() , 0.0f)); //el primer parametro es el largo y el segundo el ancho
				ImGui::PopStyleColor();
				ImGui::Text("%.2f / %.2f ", c_controller->life ,c_controller->maxLife);

				//Madness
				TCompMadnessController* madness_controller = e_player->get<TCompMadnessController>();
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.45f, 0.13f, 0.49f, 1.0f));
				ImGui::ProgressBar(madness_controller->getRemainingMadness() / c_controller->getMaxMadness(), ImVec2(GameController.getMadnessBarSize(), 0.0f));
				ImGui::PopStyleColor();
				ImGui::Text("%.2f / %.2f ", madness_controller->getRemainingMadness(), c_controller->getMaxMadness());

		  //Combo window
		  TCompTeleport* c_tp = e_player->get<TCompTeleport>();
		  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.f, 1.f, 1.f, 1.0f));
		  if (c_tp->comboDone && c_tp->timeAfterTeleport < c_tp->windowTimeCombo) {
			c_tp->timeAfterTeleport = c_tp->windowTimeCombo;
		  }
		  ImGui::ProgressBar(c_tp->timeAfterTeleport / c_tp->windowTimeCombo, ImVec2(250.f, 0.0f));
		  ImGui::PopStyleColor();

		  if (c_controller->power_selected == PowerType::TELEPORT){
			ImGui::Text("Power selected: Teleport");
		  }
		  else if (c_controller->power_selected == PowerType::FIRE) {
			ImGui::Text("Power selected: Chilli");
		  }
		  else if (c_controller->power_selected == PowerType::BATTERY) {
			ImGui::Text("Power selected: Battery");
		  }

				ImGui::Text("Madness puddles cleansed: %d", GameController.getMadnessPuddles());
				ImGui::Text("Enemies defeated: %d", GameController.getEnemiesKilled());
			}
			ImGui::End();
	*/
	/*ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs;
	CHandle h_player = getEntityByName("Player");
	CEntity * e_player = h_player;
	if (e_player != nullptr) {
		TCompCharacterController* c_controller = e_player->get<TCompCharacterController>();
		if (c_controller->life <= 0) {
			window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
			float window_size_width = 300;
			float window_size_height = 55;
			ImGui::SetNextWindowPos(ImVec2(Render.width / 2 - window_size_width / 2, Render.height / 2 - window_size_height / 2));
			ImGui::SetNextWindowSize(ImVec2(window_size_width, window_size_height));
			ImGui::Begin("Game Over", nullptr, window_flags);
			ImGui::Text("GAME OVER, press key T for continue");
			ImGui::End();
		}
		if (c_controller->endGame) {
			window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
			float window_size_width = 500;
			float window_size_height = 55;
			ImGui::SetNextWindowPos(ImVec2(Render.width / 2 - window_size_width / 2, Render.height / 2 - window_size_height / 2));
			ImGui::SetNextWindowSize(ImVec2(window_size_width, window_size_height));
			ImGui::Begin("Game Over", nullptr, window_flags);
			ImGui::Text("Congratulations , the game has finished.\nPress ESC to exit the game.");
			auto& app = CApplication::get();

			if (EngineInput["exitGame_"].justPressed()) {
				DestroyWindow(app.getHandle());
			}
			ImGui::End();
		}

	}*/

}

//}



