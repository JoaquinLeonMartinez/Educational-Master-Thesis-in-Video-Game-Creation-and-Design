#ifndef INC_COMP_GROUP_H_
#define INC_COMP_GROUP_H_

#include "comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"

class CTransform;

struct TCompGroup : public TCompBase {
  VHandles handles;
  ~TCompGroup();
  void debugInMenu();
  void add(CHandle h_new_child);
  static void registerMsgs();

  void forwardDefineLocalAABB(const TMsgDefineLocalAABB& msg);
};

#endif
