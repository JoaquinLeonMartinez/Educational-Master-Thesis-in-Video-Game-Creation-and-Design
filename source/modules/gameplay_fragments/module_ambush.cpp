#include "mcv_platform.h"
#include "module_ambush.h"
#include "components/common/comp_transform.h"

bool CModuleAmbush::start()
{
  //SAVE HERE THE INITIAL INFO OF THE SCENE/SITUATION
	firstEvent();

  return true;
}


void CModuleAmbush::stop()
{
  //JUST IN CASE SOMETHING IS NEEDED HERE
}

void CModuleAmbush::enemyKilled(EntityType e) {
  if (e == EntityType::SUSHI) {
    sushisKilled++;
  }

  if (sushisKilled == 1) {
	
  }
  else if (sushisKilled == 3) {
   
  }
}

void CModuleAmbush::renderInMenu()
{
    ImGui::Text("Ambush gameplay fragment:");
    ImGui::LabelText("thermostats activates: ","%d", termostatosActives);
}

void CModuleAmbush::firstEvent() {
	GameController.playAnnouncement("event:/UI/Announcements/Announcement10");
	termostatosActives++;
	Scripting.execActionDelayed("defrost1()", 0.0);
	Scripting.execActionDelayed("setAmbient(0.025)", 0.0);
	Scripting.execActionDelayed("changeLightsIntensityJoint(0.0)", 0.0);
	
	Scripting.execActionDelayed("apagarEncenderLuz(true)", 0.0);
	
	
	Scripting.execActionDelayed("setAmbient(0.300)", 6.0);
	Scripting.execActionDelayed("changeLightsIntensityJoint(8.0)", 6.0);
	
	Scripting.execActionDelayed("apagarEncenderLuz(false)", 6.0);
	
	//Scripting.execActionDelayed("setTransformObject_(\"Box009\",VEC3(71.813,2.501,93.810),-115,0,-49)",5.0);
	//Scripting.execActionDelayed("activePlatformByName(\"Box009\")",0.1); Ya no se mueve la zanahoria


	Scripting.execActionDelayed("wakeUpSuishisSecondFloor()", 0.1);
  //CONGELAR ENEMIGOS Y PLAYER
  //CINEMATICA
  //ACTIVAR PLATAFORMAS(SUELO)
  //DESCONGELAR ENEMIGOS Y PLAYER

	//Scripting.runScript(AMBUSH_EVENT);
}

void CModuleAmbush::secondEvent() {
	Scripting.execActionDelayed("defrost2()", 0.0); 
	Scripting.execActionDelayed("on_ambush()", 0.0); 
	Scripting.execActionDelayed("setAmbient(0.025)", 0.0);
	Scripting.execActionDelayed("changeLightsIntensityJoint(0.0)", 0.0);
	Scripting.execActionDelayed("childAppears(\"MISION_7\",true,true,0.0,1.25)", 0.1);
	Scripting.execActionDelayed("apagarEncenderLuz(true)", 0.0);
	Scripting.execActionDelayed("setAmbient(0.300)", 4.0);
	Scripting.execActionDelayed("changeLightsIntensityJoint(8.0)", 4.0);
	
	Scripting.execActionDelayed("apagarEncenderLuz(false)", 4.0);

  //Scripting.execActionDelayed("on_GPUdeleteScene(\"data/scenes/mapa_congelados.json\")", 0.0);
  //Scripting.execActionDelayed("on_load_gpu_scene(\"data/scenes/mapa_carnes.json\")", 8.0);
  //Scripting.execActionDelayed("on_load_gpu_scene(\"data/particles/hot_particles.json\")", 9.0);
	//descongelar suishis finales
  //CONGELAR ENEMIGOS Y PLAYER
  //CINEMATICA
  //ACTIVAR "BOMBARDEO"
  //DESCONGELAR ENEMIGOS Y PLAYER
}

int CModuleAmbush::getThermostatsActivates() {
	return termostatosActives;
}


