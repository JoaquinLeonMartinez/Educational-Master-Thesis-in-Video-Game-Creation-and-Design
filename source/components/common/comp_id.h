#pragma once

#include "comp_base.h"
#include <unordered_map>
#include "entity/entity.h"
 
class TCompID : public TCompBase {

	DECL_SIBLING_ACCESS();

  int _id;

public:
  static std::unordered_map< int, CHandle > all_ids;
  const int getId() const { return _id; }
  void setId(const int id);

  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
};