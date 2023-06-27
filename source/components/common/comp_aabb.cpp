#include "mcv_platform.h"
#include "comp_aabb.h"
#include "entity/common_msgs.h"
#include "resources/resource.h"
#include "render/meshes/mesh.h"
#include "render/render.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("abs_aabb", TCompAbsAABB);
DECL_OBJ_MANAGER("local_aabb", TCompLocalAABB);

void TCompAABB::load(const json& j, TEntityParseContext& ctx) {
  if (j.is_object()) {
    Center = loadVEC3( j, "center" );
    Extents = loadVEC3( j, "half_size" );
  }
}

void TCompAABB::debugInMenu() {
  ImGui::DragFloat3("center", &Center.x, 0.01f, -25.0f, 25.0f);
  ImGui::DragFloat3("half_size", &Extents.x, 0.01f, 0.0f, 25.0f);
}

void TCompAABB::renderDebug() {
  drawWiredAABB(Center, Extents, MAT44::Identity, VEC4(1, 0, 0, 1));
}

void TCompAABB::updateFromSiblingsLocalAABBs(CHandle h_entity) {
  // Time to update our AbsAABB based on the sibling components
  CEntity* e = h_entity;
  assert(e);
  // Start by computing the local aabb
  e->sendMsg(TMsgDefineLocalAABB{ this });
}

// Model * ( center +/- halfsize ) = model * center + model * half_size
AABB getRotatedBy(AABB src, const MAT44 &model) {
  AABB new_aabb;
  new_aabb.Center = VEC3::Transform(src.Center, model);
  new_aabb.Extents.x = src.Extents.x * fabsf(model(0, 0))
    + src.Extents.y * fabsf(model(1, 0))
    + src.Extents.z * fabsf(model(2, 0));
  new_aabb.Extents.y = src.Extents.x * fabsf(model(0, 1))
    + src.Extents.y * fabsf(model(1, 1))
    + src.Extents.z * fabsf(model(2, 1));
  new_aabb.Extents.z = src.Extents.x * fabsf(model(0, 2))
    + src.Extents.y * fabsf(model(1, 2))
    + src.Extents.z * fabsf(model(2, 2));
  return new_aabb;
}

// ------------------------------------------------------
void TCompAbsAABB::onCreate(const TMsgEntityCreated&) {
  updateFromSiblingsLocalAABBs(CHandle(this).getOwner());
  TCompTransform* c_trans = get<TCompTransform>();
  AABB::Transform(*this, c_trans->asMatrix());
}

void TCompAbsAABB::registerMsgs() {
  DECL_MSG(TCompAbsAABB, TMsgEntityCreated, onCreate);
}

// ------------------------------------------------------
// Updates AbsAABB from LocalAABB and CompTransform
void TCompLocalAABB::update(float dt) {
  PROFILE_FUNCTION("LocalAABB");
  const TCompTransform *in_tmx = get< TCompTransform >();
  TCompAbsAABB *abs_aabb = get<TCompAbsAABB>();

  // AbsAABB = transform( LocalAABB )
  if (in_tmx && abs_aabb)
    *(AABB*)abs_aabb = getRotatedBy(*this, in_tmx->asMatrix());

  int a = 0;
}

void TCompLocalAABB::onCreate(const TMsgEntityCreated&) { 
  updateFromSiblingsLocalAABBs(CHandle(this).getOwner());
}

void TCompLocalAABB::renderDebug() {
  const TCompTransform *in_tmx = get< TCompTransform >();
  drawWiredAABB(Center, Extents, in_tmx->asMatrix(), VEC4(1, 1, 0, 1));
}

void TCompLocalAABB::onGroupCreate(const TMsgEntitiesGroupCreated&) { 
  updateFromSiblingsLocalAABBs(CHandle(this).getOwner());
}

void TCompLocalAABB::registerMsgs() {
  DECL_MSG(TCompLocalAABB, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompLocalAABB, TMsgEntitiesGroupCreated, onGroupCreate);
}