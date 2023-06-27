#ifndef INC_RAGDOLL_H_
#define INC_RAGDOLL_H_

#include "PxPhysicsAPI.h"
struct TRagdollCore;
struct TRagdollBoneCore;

struct TRagdoll {
  struct TRagdollBone {
    TRagdollBoneCore * core;

    physx::PxRigidDynamic* actor;
    physx::PxJoint* parent_joint;

    static const int MAX_RAGDOLL_BONE_CHILDREN = 128;
    int children_idxs[MAX_RAGDOLL_BONE_CHILDREN];

    int num_children;
    int idx;
    int parent_idx;

    TRagdollBone() : num_children(0) {}

  };

  static const int MAX_RAGDOLL_BONES= 128;
  int num_bones;
  TRagdollBone bones[MAX_RAGDOLL_BONES];

  TRagdollCore* core;

  bool created;

  TRagdoll() :created(false), num_bones(0){};

};


#endif
