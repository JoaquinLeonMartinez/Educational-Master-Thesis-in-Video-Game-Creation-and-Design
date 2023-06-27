#include "mcv_platform.h"
#include "engine.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_render.h"
#include "components/objects/comp_interruptor.h"


DECL_OBJ_MANAGER("comp_interruptor", TCompInterruptor);

void TCompInterruptor::debugInMenu() {



}

void TCompInterruptor::load(const json& j, TEntityParseContext& ctx) {
	nombre = j.value("nombre", nombre);
	elementoAmover = j.value("elementoAmover", elementoAmover);
}

void TCompInterruptor::registerMsgs() {
	DECL_MSG(TCompInterruptor, TMsgDamage, onActivate);
}

void TCompInterruptor::onActivate(const TMsgDamage & msg) {
	if (msg.damageType == MELEE) {
		//llamar a script activateAlgo y nombre de lo que se activa
		if(!isOn){
			//cambiar por nombres que nos pasen en el mapa
			if (nombre.compare("panelRampa") == 0) {
				CHandle entity = getEntityByName("golem2");
				GameController.setPauseEnemyByHandle(entity,true);
			 	Scripting.execActionDelayed("activePlatformByName(\"Box040\")", 0.0);
			    GameController.resetCamera();
				GameController.wakeUpWinds();
				execDelayedAction("on_cinematic(true)", 0.0);
				execDelayedAction("on_lock_camera3(false)", 0.0);
				execDelayedAction("on_active_enemies(false)",0.0);
				execDelayedAction("on_blending_camera(\"CameraPanel003\", 5,\"linear\")", 0.5);
				execDelayedAction("on_blending_camera(\"PlayerCamera\", 6,\"linear\")", 8);
				execDelayedAction("playAnnouncement(\"event:/UI/Announcements/Announcement3\")", 0.0);
				execDelayedAction("on_lock_camera3(true)", 14);
				execDelayedAction("on_cinematic(false)", 14);
				execDelayedAction("on_active_enemies(true)", 13.0);
				execDelayedAction("setPauseEnemyName(\"golem2\",false)", 13);
				execDelayedAction("setNotThrowCupcake(\"golem2\",false)", 13);
				Scripting.execAction("wakeUpWinds()");
                AudioEvent audio = EngineAudio.playEvent("event:/Music/Ambience_Props/Panel/Panel_Break");
                TCompTransform* c_trans = get<TCompTransform>();
                audio.set3DAttributes(*c_trans);
			}
			/*else if (nombre.compare("panel_prueba2") == 0) {
				//nombre de plataforma a mover
				Scripting.execActionDelayed("activePlatformByName(\"zanahoria2\")", 0.0);
				//nombre de camaras para mostrar lo que vamos ha hacer
			}*/

			isOn = true;
		}
		
    CEntity* e = getEntityByName("electric_panel");
    if (e) {
      TCompRender* c_r = e->get<TCompRender>();
      c_r->is_visible = true;
      c_r->updateRenderManager();
    }
	}
}
	

