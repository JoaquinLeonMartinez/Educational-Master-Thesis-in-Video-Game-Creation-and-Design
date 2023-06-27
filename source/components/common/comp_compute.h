#pragma once

#include "comp_base.h"
#include "entity/entity.h"

class CComputeShader;
class CGPUBuffer;
struct TCompBuffers;
 
struct TCompCompute : public TCompBase {

  struct TExecution {
    enum eCmd {
      RUN_COMPUTE,
      SWAP_BUFFERS,
    };
    eCmd                  cmd = RUN_COMPUTE;
    const CComputeShader* compute = nullptr;
    uint32_t              sizes[3];
    bool                  sets_num_instances = false;

    bool                  is_indirect = false;
    std::string           indirect_buffer_name;
    uint32_t              indirect_buffer_offset = 0;

    std::string           swap_bufferA;
    std::string           swap_bufferB;

    void debugInMenu();
    bool getDispatchArgs(uint32_t* args);
    void load(const json& j);
    void bindArguments(TCompBuffers* c_buffers);
    void run(TCompBuffers* c_buffers);
  };

  std::vector< TExecution > executions;

  void loadOneExecution(const json& j);

  bool paused = false;
  bool auto_paused = false;

public:

  void load(const json& j, TEntityParseContext& ctx);
  void debugInMenu();
  void update(float dt);

  DECL_SIBLING_ACCESS();
};