#ifndef INC_RENDER_MESHES_INSTANCED_MESH_H_
#define INC_RENDER_MESHES_INSTANCED_MESH_H_

#include "mesh.h"

// -------------------------------------------------------------------
// I' will hold the instances data. instanced_mesh is
// a pointer to the other data, the billboard
// This part is independent of the type of instance of the mesh, which has
// been moved to a .cpp file
class CMeshInstanced : public CMesh {
protected:
  const CMesh* instanced_mesh = nullptr;    // This is the teapot, billboard, ..
  size_t       num_instances_allocated_in_gpu = 0;
public:
  void configure(const json& j);
  void renderGroup(uint16_t group_idx, uint16_t instanced_group_idx) const override;
  void reserveGPUInstances(size_t new_max_instances);
  void setInstancesData(const void* data, size_t total_instances, size_t bytes_per_instance );
  void setGroupSize(uint32_t num_subgroup, uint32_t new_size_for_subgroup);
};

#endif
