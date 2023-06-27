#pragma once

#include "comp_base.h"
#include "entity/entity.h"

class CGPUBuffer;
 
struct TCompBuffers : public TCompBase {

  std::vector< CGPUBuffer* >     gpu_buffers;
  std::vector< CCteBufferBase* > cte_buffers;

public:

  ~TCompBuffers();

  void load(const json& j, TEntityParseContext& ctx);

  void debugInMenu();

  CGPUBuffer* getBufferByName(const char* name);
  CCteBufferBase* getCteByName(const char* name);

};