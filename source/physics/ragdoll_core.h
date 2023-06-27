#ifndef INC_RAGDOLL_CORE_H_
#define INC_RAGDOLL_CORE_H_

struct TRagdollBoneCore {
  std::string bone;
  std::string parent_bone;
  float height;
  float radius;
  TRagdollBoneCore* parent_core;
  int instance_idx;
};


struct TRagdollCore{
  std::vector<TRagdollBoneCore> ragdoll_bone_cores;
};


#endif
