#include "mcv_platform.h"
#include "comp_skeleton.h"
#include "cal3d/cal3d.h"
#include "game_core_skeleton.h"
#include "render/primitives.h"      // for the max_skeletons_ctes
#include "components/common/comp_transform.h"
#include "render/primitives.h"      // for the max_skeletons_ctes

// Changed name from skeleton to force to be parsed before the comp_mesh
DECL_OBJ_MANAGER("armature", TCompSkeleton);

// ---------------------------------------------------------------------------------------
// Cal2DX conversions, VEC3 are the same, QUAT must change the sign of w
CalVector DX2Cal(VEC3 p) {
  return CalVector(p.x, p.y, p.z);
}
CalQuaternion DX2Cal(QUAT q) {
  return CalQuaternion(q.x, q.y, q.z, -q.w);
}
VEC3 Cal2DX(CalVector p) {
  return VEC3(p.x, p.y, p.z);
}
QUAT Cal2DX(CalQuaternion q) {
  return QUAT(q.x, q.y, q.z, -q.w);
}
MAT44 Cal2DX(CalVector trans, CalQuaternion rot) {
  return
    MAT44::CreateFromQuaternion(Cal2DX(rot))
    * MAT44::CreateTranslation(Cal2DX(trans))
    ;
}

// ---------------------------------------------------------------------------------------
void TCompSkeleton::load(const json& j, TEntityParseContext& ctx) {
	_unscaledTime = j.value("_unscaledTime", _unscaledTime);

  std::string src = j["src"];
  auto core = Resources.get(src)->as<CGameCoreSkeleton>();

  model = new CalModel((CalCoreModel*)core);

  for (int i = 0; i < model->getCoreModel()->getCoreAnimationCount(); i++) {

	  auto core_anim = model->getCoreModel()->getCoreAnimation(i);
	  if (core_anim)
		  stringAnimationIdMap[core_anim->getName()] = i;
  }


  actualCycleAnimId[1] = -1;
  actualCycleAnimId[0] = 0;

  model->getMixer()->blendCycle(0, 1.0f, 0.f);

  // Do a time zero update just to have the bones in a correct place
  model->update(0.f);

}

TCompSkeleton::TCompSkeleton()
  : cb_bones(CTE_BUFFER_SLOT_SKIN_BONES)
{
  bool is_ok = cb_bones.create("Bones");
  assert(is_ok);
}

TCompSkeleton::~TCompSkeleton() {
  if (model)
    delete model;
  cb_bones.destroy();
  model = nullptr;
}

void TCompSkeleton::update(float dt) {
  PROFILE_FUNCTION("updateSkel");
  assert(model);
  TCompTransform* tmx = get<TCompTransform>();
  if(tmx != NULL){
	  VEC3 pos = tmx->getPosition();
	  QUAT rot = tmx->getRotation();
	  model->getMixer()->setWorldTransform(DX2Cal(pos), DX2Cal(rot));
	  float _dt;
	  if (_unscaledTime) {
		  _dt = Time.delta_unscaled;
	  }
	  else {
		  _dt = Time.delta;
	  }
	  model->update(_dt);
	  VEC3 root_motion = Cal2DX(model->getMixer()->getAndClearDeltaRootMotion());
	  tmx->setPosition(pos + root_motion);
  }
}

void TCompSkeleton::debugInMenu() {
  static int anim_id = 0;
  static float in_delay = 0.3f;
  static float out_delay = 0.3f;
  static bool auto_lock = false;

  // Play aacton/cycle from the menu
  ImGui::DragInt("Anim Id", &anim_id, 0.1f, 0, model->getCoreModel()->getCoreAnimationCount() - 1);
  auto core_anim = model->getCoreModel()->getCoreAnimation(anim_id);
  if (core_anim)
    ImGui::Text("%s", core_anim->getName().c_str());
  ImGui::DragFloat("In Delay", &in_delay, 0.01f, 0, 1.f);
  ImGui::DragFloat("Out Delay", &out_delay, 0.01f, 0, 1.f);
  ImGui::Checkbox("Auto lock", &auto_lock);
  if (ImGui::SmallButton("As Cycle")) {
    model->getMixer()->blendCycle(anim_id, 1.0f, in_delay);
  }
  if (ImGui::SmallButton("As Action")) {
    model->getMixer()->executeAction(anim_id, in_delay, out_delay, 1.0f, auto_lock);
  }

  // Dump Mixer
  auto mixer = model->getMixer();
  for (auto a : mixer->getAnimationActionList()) {
    ImGui::Text("Action %s S:%d W:%1.2f Time:%1.4f/%1.4f"
      , a->getCoreAnimation()->getName().c_str()
      , a->getState()
      , a->getWeight()
      , a->getTime()
      , a->getCoreAnimation()->getDuration()
    );
    ImGui::SameLine();
    if (ImGui::SmallButton("X")) {
      auto core = (CGameCoreSkeleton*)model->getCoreModel();
      int id = core->getCoreAnimationId(a->getCoreAnimation()->getName());
      if (a->getState() == CalAnimation::State::STATE_STOPPED)
        mixer->removeAction(id);
      else
        a->remove(out_delay);
    }
  }

  for (auto a : mixer->getAnimationCycle()) {
    ImGui::PushID(a);
    ImGui::Text("Cycle %s S:%d W:%1.2f Time:%1.4f"
      , a->getCoreAnimation()->getName().c_str()
      , a->getState()
      , a->getWeight()
      , a->getCoreAnimation()->getDuration()
    );
    ImGui::SameLine();
    if (ImGui::SmallButton("X")) {
      auto core = (CGameCoreSkeleton*)model->getCoreModel();
      int id = core->getCoreAnimationId(a->getCoreAnimation()->getName());
      mixer->clearCycle(id, out_delay);
    }
    ImGui::PopID();
  }

  if (mixer->getAnimationCycle().size() == 2) {
    static bool unit_weight = false;
    ImGui::Checkbox("Unit Weight", &unit_weight);
    if (unit_weight) {
      static float w1 = 0.5f;
      auto anim1 = mixer->getAnimationCycle().front();
      auto anim2 = mixer->getAnimationCycle().back();
      float wsmall = 1e-3f;
      if (ImGui::DragFloat("Weight", &w1, 0.005f, wsmall, 1.0f - wsmall)) {
        int anim1_id = model->getCoreModel()->getCoreAnimationId(anim1->getCoreAnimation()->getName());
        int anim2_id = model->getCoreModel()->getCoreAnimationId(anim2->getCoreAnimation()->getName());
        model->getMixer()->blendCycle(anim1_id, w1, 0.1f);
        model->getMixer()->blendCycle(anim2_id, 1.0f - w1, 0.1f);
      }
    }
  }



  // Show Skeleton Resource
  if (ImGui::TreeNode("Core")) {
    auto core_skel = (CGameCoreSkeleton*)model->getCoreModel();
    if (core_skel)
      core_skel->renderInMenu();
    ImGui::TreePop();
  }

}

void TCompSkeleton::renderDebug() {
  assert(model);

  VEC3 lines[MAX_SUPPORTED_BONES][2];
  int nrLines = model->getSkeleton()->getBoneLines(&lines[0][0].x);
  TCompTransform* transform = get<TCompTransform>();
  float scale = transform->getScale();
  for (int currLine = 0; currLine < nrLines; currLine++)
    drawLine(lines[currLine][0] * scale, lines[currLine][1] * scale, VEC4(1, 1, 1, 1));

  // Show list of bones
  auto mesh = Resources.get("axis.mesh")->as<CMesh>();
  auto core = (CGameCoreSkeleton*)model->getCoreModel();
  auto& bones_to_debug = core->bone_ids_to_debug;
  for (auto it : bones_to_debug) {
    CalBone* cal_bone = model->getSkeleton()->getBone(it);
    QUAT rot = Cal2DX(cal_bone->getRotationAbsolute());
    VEC3 pos = Cal2DX(cal_bone->getTranslationAbsolute());
    MAT44 mtx;
    mtx = MAT44::CreateFromQuaternion(rot) * MAT44::CreateTranslation(pos);
    drawMesh(mesh, mtx, VEC4(1, 1, 1, 1));
  }
}

void TCompSkeleton::updateCtesBones() {
  PROFILE_FUNCTION("updateCtesBones");

  // Pointer to the first float of the array of matrices
  float* fout = &cb_bones.Bones[0]._11;

  CalSkeleton* skel = model->getSkeleton();
  auto& cal_bones = skel->getVectorBone();
  assert(cal_bones.size() < MAX_SUPPORTED_BONES);

  // For each bone from the cal model
  for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
    CalBone* bone = cal_bones[bone_idx];

    const CalMatrix& cal_mtx = bone->getTransformMatrix();
    const CalVector& cal_pos = bone->getTranslationBoneSpace();

    *fout++ = cal_mtx.dxdx;
    *fout++ = cal_mtx.dydx;
    *fout++ = cal_mtx.dzdx;
    *fout++ = 0.f;
    *fout++ = cal_mtx.dxdy;
    *fout++ = cal_mtx.dydy;
    *fout++ = cal_mtx.dzdy;
    *fout++ = 0.f;
    *fout++ = cal_mtx.dxdz;
    *fout++ = cal_mtx.dydz;
    *fout++ = cal_mtx.dzdz;
    *fout++ = 0.f;
    *fout++ = cal_pos.x;
    *fout++ = cal_pos.y;
    *fout++ = cal_pos.z;
    *fout++ = 1.f;
  }

  cb_bones.updateGPU();
}

void TCompSkeleton::changeCyclicAnimation(int anima1Id, float speed, int anima2Id, float weight, float in_delay, float out_delay) {
	model->getMixer()->clearCycle(actualCycleAnimId[0], out_delay);
	if (actualCycleAnimId[1] != -1) {
		model->getMixer()->clearCycle(actualCycleAnimId[1], out_delay);
	}

	model->getMixer()->blendCycle(anima1Id, weight, in_delay);
	if (anima2Id != -1) {
		model->getMixer()->blendCycle(anima2Id, 1.f - weight, in_delay);
	}

	actualCycleAnimId[0] = anima1Id;
	actualCycleAnimId[1] = anima2Id;

	model->getMixer()->setTimeFactor(speed);
}
//ejecucion de animacion de accion
void TCompSkeleton::executeActionAnimation(int animaId, float speed, float in_delay, float out_delay) {

	bool auto_lock = false;
	for (auto a : model->getMixer()->getAnimationActionList()) {
		a->remove(out_delay);
	}
	model->getMixer()->executeAction(animaId, in_delay, out_delay, 1.0f, auto_lock);

	if (speed != 1.0f) {
		std::list<CalAnimationAction *>::iterator iteratorAnimationAction;
		iteratorAnimationAction = model->getMixer()->getAnimationActionList().begin();
		while (iteratorAnimationAction != model->getMixer()->getAnimationActionList().end())
		{
			(*iteratorAnimationAction)->setTimeFactor(speed);
			iteratorAnimationAction++;
		}
	}
}
//cambio de peso en animacion
void TCompSkeleton::setCyclicAnimationWeight(float new_value) {
	cyclicAnimationWeight = new_value;
}

float TCompSkeleton::getCyclicAnimationWeight() {
	return cyclicAnimationWeight;
}

int TCompSkeleton::getAnimationIdByName(std::string animaName) {
	if (stringAnimationIdMap.find(animaName) != stringAnimationIdMap.end()) {
		return stringAnimationIdMap[animaName];
	}
	return -1;
}

bool TCompSkeleton::actionAnimationOnExecution() {

	return model->getMixer()->getAnimationActionList().size() > 0;
}

//devuelve si la animacion ciclica se esta ejecutando
bool TCompSkeleton::isExecutingCyclicAnimation(int animaId) {

	return model->getMixer()->getAnimationVector()[animaId] != NULL;
}

//devuelve si la animacion de accion se esta ejecutando
bool TCompSkeleton::isExecutingActionAnimation(std::string animaName) {

	std::list<CalAnimationAction *>::iterator iteratorAnimationAction;
	iteratorAnimationAction = model->getMixer()->getAnimationActionList().begin();
	while (iteratorAnimationAction != model->getMixer()->getAnimationActionList().end())
	{
		std::string itName = (*iteratorAnimationAction)->getCoreAnimation()->getName();
		if (itName.compare(animaName) == 0) {
			return true;
		}
		iteratorAnimationAction++;
	}
	return false;
}

float TCompSkeleton::getAnimationDuration(int animaId) {

	auto core_anima = model->getCoreModel()->getCoreAnimation(animaId);
	if (core_anima)
		return core_anima->getDuration();
	return -1.f;
}


VEC3 TCompSkeleton::getBonePositionByName(const std::string & name) {

	
	int bone_id = model->getCoreModel()->getCoreSkeleton()->getCoreBoneId(name);
	return Cal2DX(model->getSkeleton()->getBone(bone_id)->getTranslationAbsolute());
}

VEC3 TCompSkeleton::setBonePositionByName(const std::string & name) {

	
	int bone_id = model->getCoreModel()->getCoreSkeleton()->getCoreBoneId(name);
	return Cal2DX(model->getSkeleton()->getBone(bone_id)->getTranslationAbsolute());
}



VEC3 TCompSkeleton::getBonePositionById(int id) {
	VEC3 pos = Cal2DX(model->getSkeleton()->getBone(id)->getTranslationAbsolute());
	return pos;
}

void TCompSkeleton::setBonePositionById(int id, VEC3 position) {
	model->getSkeleton()->getBone(id)->setTranslation(DX2Cal(position));
}

void TCompSkeleton::clearAnimations() {
    static float out_delay = 0.3f;
    auto mixer = model->getMixer();
    for (auto a : mixer->getAnimationActionList()) {
        auto core = (CGameCoreSkeleton*)model->getCoreModel();
        int id = core->getCoreAnimationId(a->getCoreAnimation()->getName());
        mixer->removeAction(id); 
    }
}