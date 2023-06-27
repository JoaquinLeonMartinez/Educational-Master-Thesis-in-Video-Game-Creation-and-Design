#pragma once

#include "resources/resource.h"
#include "mesh_group.h"

struct CVertexDeclaration;
class CGPUBuffer;

class CMesh : public IResource {

public:

  enum eTopology {
    UNDEFINED = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    TRIANGLE_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
    LINE_STRIP = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
    TRIANGLE_STRIP = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    POINT_LIST = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST
  };

protected:

  ID3D11Buffer* vb = nullptr;
  ID3D11Buffer* ib = nullptr;
  uint32_t      num_vertices = 0;
  uint32_t      num_indices = 0;
  uint32_t      bytes_per_vertice = 0;
  DXGI_FORMAT   index_fmt = DXGI_FORMAT_UNKNOWN;
  eTopology     topology = UNDEFINED;
  const CVertexDeclaration* vertex_decl = nullptr;
  bool          is_dynamic = false;
  VMeshGroups   groups;
  AABB          aabb;

  // Use to update VB and IB
  void updateResourceFromCPU(ID3D11Resource* res, const void *new_cpu_data, size_t num_bytes_to_update, size_t max_bytes_allowed);
  void activateIndexBuffer() const;

  friend class CMeshInstanced;

public:

  ~CMesh() {
    destroy();
  }

  bool create(
      const void* vertices
    , uint32_t new_num_vertices 
    , uint32_t new_bytes_per_vertice
    , const std::string& vertex_decl_name
    , const void* indices = nullptr
    , uint32_t new_num_indices = 0
    , uint32_t new_bytes_per_indice = 0
    , eTopology = TRIANGLE_LIST
    , bool new_is_dynamic = false
    , const AABB* new_aabb = nullptr
  );
  void setGroups(const VMeshGroups& new_groups);
  const VMeshGroups& getGroups() const {
    return groups;
  }
  VMeshGroups& getGroups() {    // Non-const version
    return groups;
  }
  void destroy();
  void activate() const;
  void render() const;
  virtual void renderGroup(uint16_t group_idx, uint16_t instanced_group_idx) const;
  void renderRange(uint32_t count, uint32_t base) const;
  void renderGroupInstanced(uint16_t group_idx, uint32_t num_instances) const;
  void renderIndirect(const CGPUBuffer* buffer, uint32_t offset = 0) const;
  void activateAndRender() const;
  void renderInMenu() override;
  void updateVertsFromCPU(const void *new_cpu_data, size_t num_bytes_to_update);
  void updateIndicesFromCPU(const void *new_cpu_data, size_t num_bytes_to_update);
  bool isValid() const { return vb != nullptr && !groups.empty(); }
  const CVertexDeclaration* getVertexDecl() const {
    return vertex_decl;
  }
  const AABB& getAABB() const { return aabb; }
  void setResourcesName(const std::string& new_name);
};
