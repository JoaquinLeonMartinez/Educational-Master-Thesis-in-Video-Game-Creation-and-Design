#pragma once

#include "modules/module.h"
#include "components/common/comp_compute.h"
#include "components/common/comp_buffers.h"

class CGPUBuffer;
class CMaterial;
class CComputeShader;

class CModuleGPUCulling : public IModule {

  // -----------------------------------------------
  // All objects in the scene, input of the culling
  // Size MUST match the size of the gpu_instances
  // This structure is used in the GPU
  struct TObj {
    VEC3      aabb_center;    // AABB
    uint32_t  prefab_idx;     // In the array of prefabs
    VEC3      aabb_half;
    uint32_t  unique_id = -1;
    MAT44     world;          // Matrix
  };

  // -----------------------------------------------
  // This are the keys that make the instance type unique
  // This structure is NOT used in the GPU
  struct TRenderTypeID {
    const CMesh*     mesh = nullptr;
    uint32_t         group = 0;
    const CMaterial* material = nullptr;
    char             title[64];
    bool operator==(const TRenderTypeID& other) const {
      return mesh == other.mesh && group == other.group && material == other.material;
    }
  };

  // -----------------------------------------------
  // Each draw call requires this 5 uint's as arguments
  // We can command the GPU to execute a DrawIndexedInstance
  // using the args store in some GPU buffer.
  struct DrawIndexedInstancedArgs {
    uint32_t indexCount;
    uint32_t instanceCount;     // at offset 4
    uint32_t firstIndex;
    uint32_t firstVertex;
    uint32_t firstInstance;
  };

  // -----------------------------------------------
  // This struct is used by the GPU
  struct TPrefab {
    static const uint32_t max_render_types_per_prefab = 6;
    CHandle   id;
    uint32_t  lod_prefab = -1;
    float     lod_threshold = 1e5;
    uint32_t  num_objs = 0;             // Number of instances of this type
    uint32_t  num_render_type_ids = 0;  // How many parts we must add
    uint32_t  total_num_objs = 0;       // Takes into account if another prefab is using me to render a LOD
    uint32_t  render_type_ids[max_render_types_per_prefab] = { 0,0,0,0,0,0 };
  };

  // -----------------------------------------------
  // Upload to the GPU so it can fill the num_instances
  // The CPU will fill the args, and the base
  struct TDrawData {
    DrawIndexedInstancedArgs args;      // 5 ints
    uint32_t                 base = 0;  // Where in the array of culled_instances we can start adding objs
    uint32_t                 max_instances = 0;  // How many instances of this render type we will ever use
    uint32_t                 dummy = 0;
  };

  // -----------------------------------------------
  struct TCullingPlanes {
    VEC4  planes[6];
    VEC3  CullingCameraPos;
    float dummy;
  };

  // -----------------------------------------------
  struct TCtesInstancing {
    uint32_t total_num_objs = 0;
    uint32_t instance_base = 0;
    uint32_t instancing_padding[2];
  };

  // -----------------------------------------------
  std::vector< TObj >           objs;
  std::vector< TPrefab >        prefabs;
  std::vector< TDrawData >      draw_datas;
  std::vector< TRenderTypeID >  render_types;
  bool                          is_dirty = false;
  bool                          show_debug = true;


  CCamera                       culling_camera;
  TCullingPlanes                culling_planes;
  TCtesInstancing               ctes_instancing;

  std::string                   entity_camera_name;
  CHandle                       h_camera;

  // -----------------------------------------------
  TCompCompute                  comp_compute;
  TCompBuffers                  comp_buffers;
  CGPUBuffer*                   gpu_objs = nullptr;
  CGPUBuffer*                   gpu_draw_datas = nullptr;
  CGPUBuffer*                   gpu_prefabs = nullptr;
  CCteBufferBase*               gpu_ctes_instancing = nullptr;
  uint32_t                      max_objs = 0;
  uint32_t                      max_render_types = 0;
  uint32_t                      max_prefabs = 0;

  void updateCamera();
  void updateCullingPlanes(const CCamera& camera);
  void clearRenderDataInGPU();
  void preparePrefabs();
  uint32_t registerPrefab(CHandle new_id);
  uint32_t addPrefabInstance(CHandle new_id);
  uint32_t addRenderType(const TRenderTypeID& new_render_type);
  void setPrefabLod(uint32_t high_prefab_idx, uint32_t low_prefab_idx, float threshold);

public:

  CModuleGPUCulling() : IModule("gpu_culling") {}

  void addToRender(CHandle prefab, const AABB aabb, const MAT44 world);

  // -----------------------------------------------
  bool start() override;
  void update( float dt ) override;
  void stop() override;
  void run();
  void runWithCustomCamera(const CCamera& light_camera);
  void renderDebug() override;
  void renderInMenu() override;
  void renderCategory(eRenderCategory category);
  void setDirty() { is_dirty = true; }
  void updateObjData(int idx, CHandle entity);
  int getObjSize();
  void parseEntities(const std::string& filename, TEntityParseContext& ctx);
  void parseProducts(const std::string& filename, TEntityParseContext& ctx);
  void createPrefabProducts();
  void clear();


  //for the products
  int                           first_prod_index = 5000;
  int                           last_prod_index = -1;
};


