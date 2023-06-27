#include "mcv_platform.h"
#include "comp_group.h"
#include "comp_hierarchy.h"

DECL_OBJ_MANAGER("group", TCompGroup);

void TCompGroup::debugInMenu() {
  for (auto h : handles)
    h.debugInMenu();
}

TCompGroup::~TCompGroup() {
  for (auto h : handles)
    h.destroy();
}

void TCompGroup::forwardDefineLocalAABB(const TMsgDefineLocalAABB& msg) {
  // Update msg.aabb using all children's aabb
  for (auto h : handles) {
    // Get local aabb of child
    AABB child_local_aabb;
    h.sendMsg(TMsgDefineLocalAABB{ &child_local_aabb });

    // If the child has a comp_hierarchy...
    CEntity* e_child = h;
    TCompHierarchy* c_hierarchy = e_child->get<TCompHierarchy>();
    if (!c_hierarchy)
      continue;

    // Transform his local space into my local space
    AABB child_in_my_local_space;
    child_local_aabb.Transform(child_in_my_local_space, c_hierarchy->asMatrix());
      
    // And combine it with the original msg aabb
    AABB::CreateMerged(*msg.aabb, *msg.aabb, child_in_my_local_space);
  }
}

void TCompGroup::registerMsgs() {
  // Un comment to enable aabb of parent includes aabb of all children on creation
  // DECL_MSG(TCompGroup, TMsgDefineLocalAABB, forwardDefineLocalAABB);
}

void TCompGroup::add(CHandle h_new_child) {
  handles.push_back(h_new_child);
  // The new child should not have another parent.Or we should tell him he is no longer his child...
  assert(!h_new_child.getOwner().isValid());
  // Using the handle owner to set his parent my entity
  h_new_child.setOwner(CHandle(this).getOwner());
}
