#include "mcv_platform.h"
#include "comp_teleport.h"
#include "components/controllers/character/comp_character_controller.h"
#include "components/controllers/camera/comp_aim_controller.h"
#include "components/common/physics/comp_collider.h"
#include "components/common/comp_transform.h"
#include "components/powers/comp_madness.h"
#include "components/common/comp_render.h"
#include "engine.h"
#include "input/module_input.h"
#include "modules/module_camera_mixer.h"
#include "ui/controllers/ui_menu_controller.h"
#include "ui/widgets/ui_button.h"
#include "ui/module_ui.h"
using namespace physx;

DECL_OBJ_MANAGER("comp_teleport", TCompTeleport);


void TCompTeleport::update(float delta)
{
  timeAfterTeleport += delta;
  timeAfterTeleportUI -= delta;
}

void TCompTeleport::rayCast() {
  if (!h_player.isValid()) {
	  h_player = GameController.getPlayerHandle();
  }

  if (!h_camera.isValid()) {
    h_camera = getEntityByName("PlayerCamera");
  }

  CEntity* e_camera = h_camera;
  TCompTransform* c_trans_cam = e_camera->get<TCompTransform>();
  //debug
  VEC3 pos = c_trans_cam->getPosition();
  VEC3 direction = c_trans_cam->getFront();
  VEC3 aux1 = c_trans_cam->getLeft();
  VEC3 aux2 = c_trans_cam->getUp();
  direction.Normalize();
  source = pos;

  auto scene = EnginePhysics.getScene();
  PxQueryFilterData filter_data = PxQueryFilterData();
  filter_data.data.word0 = EnginePhysics.TpRay;

  // [in] Define what parts of PxRaycastHit we're interested in
  const PxHitFlags outputFlags =
    PxHitFlag::eDISTANCE
    | PxHitFlag::ePOSITION
    | PxHitFlag::eNORMAL
    ;

  PxRaycastBuffer hit;
  PxRaycastHit hitBuffer[10];
  hit = PxRaycastBuffer(hitBuffer, 10);
  PxReal _maxDistance = 40.f;
  bool colDetected = scene->raycast(
    VEC3_TO_PXVEC3(pos),
    VEC3_TO_PXVEC3(direction),
    _maxDistance,
    hit,
    outputFlags,
    filter_data
  );

  if (!hasCollided(colDetected, hit)) {
    VEC3 new_pos = pos + aux1 * 0.1f;
    bool colDetected = scene->raycast(
      VEC3_TO_PXVEC3(new_pos),
      VEC3_TO_PXVEC3(direction),
      _maxDistance,
      hit,
      outputFlags,
      filter_data
    );
    if (!hasCollided(colDetected, hit)) {
      VEC3 new_pos = pos - aux1 * 0.1f;
      bool colDetected = scene->raycast(
        VEC3_TO_PXVEC3(pos),
        VEC3_TO_PXVEC3(direction),
        _maxDistance,
        hit,
        outputFlags,
        filter_data
      );
      if (!hasCollided(colDetected, hit)) {
        VEC3 new_pos = pos - aux2 * 0.1f;
        bool colDetected = scene->raycast(
          VEC3_TO_PXVEC3(pos),
          VEC3_TO_PXVEC3(direction),
          _maxDistance,
          hit,
          outputFlags,
          filter_data
        );
        if (!hasCollided(colDetected, hit)) {
          return;
        }
      }
    }
  }
  
  
  //If we're here we have missed
  EngineAudio.playEvent("event:/Character/Powers/Scanner/Scan_Fail");
}

bool TCompTeleport::hasCollided(bool colDetected, PxRaycastBuffer hit) {
  if (colDetected) {
    int closestIdx = -1;
    float closestDist = 1000.0f;
    dbg("Number of hits: %i \n", hit.getNbAnyHits());
    for (int i = 0; i < hit.getNbAnyHits(); i++) {
      if (hit.getAnyHit(i).distance <= closestDist) {
        closestDist = hit.getAnyHit(i).distance;
        closestIdx = i;
      }
    }

    if (closestIdx != -1) {
      CHandle hitCollider;
      PxShape* colShape;
      for (int i = 0; i < hit.getAnyHit(closestIdx).actor->getNbShapes(); i++) {
        hit.getAnyHit(closestIdx).actor->getShapes(&colShape, 1, i);
        PxFilterData col_filter_data = colShape->getSimulationFilterData();
        if (col_filter_data.word0 & EnginePhysics.TeleportableObj) {
          hitCollider.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
          if (hitCollider.isValid()) {
            CEntity* candidate = hitCollider.getOwner();
            //dbg("el candidato obj es valido nombre = %s  \n", candidate->getName());
            TCompMadnessController* m_c = get<TCompMadnessController>();
            m_c->generateMadness(PowerType::TELEPORT);
            teleportWithObject(PXVEC3_TO_VEC3(hit.getAnyHit(closestIdx).position));
            EngineAudio.playEvent("event:/Character/Powers/Scanner/Scan_Success");
			
            return true;
          }
        }
        else if (col_filter_data.word0 & EnginePhysics.TeleportableEnemy) {
          hitCollider.fromVoidPtr(hit.getAnyHit(closestIdx).actor->userData);
          if (hitCollider.isValid()) {
            CEntity* candidate = hitCollider.getOwner();
            //dbg("el candidato es valido nombre = %s  \n", candidate->getName());
            TCompMadnessController* m_c = get<TCompMadnessController>();
            m_c->generateMadness(PowerType::TELEPORT);
            teleportWithEnemy(PXVEC3_TO_VEC3(hit.getAnyHit(closestIdx).position));
            EngineAudio.playEvent("event:/Character/Powers/Scanner/Scan_Success");
			
            return true;
          }
        }
      }
    }

  }
  return false;
}

void TCompTeleport::teleportWithEnemy(VEC3 candidate_position) {
  timeAfterTeleport = 0.f;
  comboDone = false;

  CEntity* e_player = (CEntity *)h_player;
  TCompCollider* c_collider_player = e_player->get<TCompCollider>();

  VEC3 dir = candidate_position - PXVEC3_TO_VEC3(c_collider_player->controller->getPosition());
  dir.Normalize();

  VEC3 final_pos = candidate_position - dir;

  PxExtendedVec3 final_pos_px = VEC3_TO_PXEXVEC3(final_pos);

  //cambiar el collider y no la transform
  //cambiar posiciones
  c_collider_player->controller->setPosition(final_pos_px);
  
  //UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("card_"));
  //boton->setCurrentState("selected");
  
  manageCameras();

  
}

void TCompTeleport::teleportWithObject(VEC3 candidate_position) {
  timeAfterTeleport = 0.f;
  comboDone = false;
  
  //UI::CButton* boton = dynamic_cast<UI::CButton*>(Engine.getUI().getWidgetByAlias("card_"));
  //boton->setCurrentState("selected");
  
  //change positions between object and player
  CEntity* e_player = (CEntity *)h_player;
  TCompCollider* c_collider_player = e_player->get<TCompCollider>();

  VEC3 dir = candidate_position - PXVEC3_TO_VEC3(c_collider_player->controller->getPosition());
  dir.Normalize();

  VEC3 final_pos = candidate_position - dir;

  PxExtendedVec3 final_pos_px = VEC3_TO_PXEXVEC3(final_pos);

  //Movemos jugador
  c_collider_player->controller->setFootPosition(final_pos_px);

  


  manageCameras();

  
}

bool TCompTeleport::canCombo() {
  return !comboDone && timeAfterTeleport <= windowTimeCombo;
}

void TCompTeleport::manageCameras() {
  CEntity* p_camera = getEntityByName("PlayerCamera");
  TCompTransform* c_pcam = p_camera->get<TCompTransform>();

  TEntityParseContext ctx;
  ctx.root_transform = *c_pcam;
  parseScene("data/prefabs/cameras/teleport_camera_fov.json", ctx);
  parseScene("data/prefabs/cameras/teleport_camera.json", ctx);

  static Interpolator::TQuadInOutInterpolator quadInt;
  CEntity* e_camera = getEntityByName("TeleportCamera");
  CEntity* e_camera_fov = getEntityByName("TeleportCameraFov");

  //BLEND TELEPORT CAMERA INSTANTLY, SO THE CAMERA DOESNT MOVE
  Engine.getCameraMixer().blendCamera(e_camera, 0.0f, &quadInt);
  Engine.getCameraMixer().blendCamera(e_camera_fov, 0.3f, &quadInt);

  //BLEND TO PLAYER CAMERA
  execDelayedAction("blendPlayerCamera()", 0.3f);
  
}

void TCompTeleport::renderDebug() {
  drawLine(source, objective, VEC4(0, 1, 0, 1));
}

void TCompTeleport::load(const json& j, TEntityParseContext& ctx) {
}