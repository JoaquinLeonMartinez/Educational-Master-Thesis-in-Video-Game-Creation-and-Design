#include "mcv_platform.h"
#include "render/render.h"
#include "render/compute/gpu_buffer.h"

bool CMesh::create(
  const void* vertices
  , uint32_t new_num_vertices
  , uint32_t new_bytes_per_vertice
  , const std::string& vertex_decl_name
  , const void* indices
  , uint32_t new_num_indices
  , uint32_t new_bytes_per_indice
  , eTopology new_topology
  , bool new_is_dynamic
  , const AABB* new_aabb
) {

  // Dynamic meshes are allowed to NOT provide initial vertex data
  assert(vertices != nullptr || new_is_dynamic);

  assert(vb == nullptr);

  is_dynamic = new_is_dynamic;

  vertex_decl = getVertexDeclByName(vertex_decl_name);
  assert(vertex_decl);
  assert(vertex_decl->bytes_per_vertex == new_bytes_per_vertice);

  // Save ctes params
  topology = new_topology;
  num_vertices = new_num_vertices;
  bytes_per_vertice = new_bytes_per_vertice;
  num_indices = new_num_indices;

  // Create Vertex buffer
  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = bytes_per_vertice * num_vertices;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  // Because we want to write from the CPU and to modify it (write, not read)
  if (is_dynamic) {
    bd.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    bd.Usage = D3D11_USAGE_DYNAMIC;
  }

  D3D11_SUBRESOURCE_DATA* addr_init_data = nullptr;
  D3D11_SUBRESOURCE_DATA init_data;
  if (vertices) {
    ZeroMemory(&init_data, sizeof(init_data));
    init_data.pSysMem = vertices;
    addr_init_data = &init_data;
  }
  HRESULT hr = Render.device->CreateBuffer(&bd, addr_init_data, &vb);
  if (FAILED(hr))
    return false;

  // Create the index buffer
  if (indices) {
    assert(num_indices != 0);
    assert(new_bytes_per_indice != 0);
   
    index_fmt = (new_bytes_per_indice == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = new_bytes_per_indice * num_indices;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    if (is_dynamic) {
      bd.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
      bd.Usage = D3D11_USAGE_DYNAMIC;
    }

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    HRESULT hr = Render.device->CreateBuffer(&bd, &InitData, &ib);
    if (FAILED(hr))
      return false;
  }
  else {
    assert(num_indices == 0);
    assert(new_bytes_per_indice == 0);
    assert(ib == nullptr);
  }

  if (groups.empty()) {
    TMeshGroup single_group;
    single_group.first_idx = 0;
    single_group.material_index = 0;
    single_group.num_indices = num_indices == 0 ? num_vertices : num_indices;
    single_group.user_material_id = 0;
    groups.push_back(single_group);
  }

  // Update aabb
  if (!new_aabb) {
    if(vertices)
      AABB::CreateFromPoints(aabb, num_vertices, (const VEC3*)vertices, bytes_per_vertice);
  }
  else
    aabb = *new_aabb;

  return true;
}

void CMesh::destroy() {
  SAFE_RELEASE(ib);
  SAFE_RELEASE(vb);
}

void CMesh::render() const {
  if (ib) 
    renderRange(num_indices, 0);
  else
    renderRange(num_vertices, 0);
}

void CMesh::renderIndirect(const CGPUBuffer* buffer, uint32_t offset ) const {
  assert(buffer);
  if (ib) {
    Render.ctx->DrawIndexedInstancedIndirect(buffer->buffer, offset);
  }
  else {
    Render.ctx->DrawInstancedIndirect(buffer->buffer, offset);
  }
}

void CMesh::renderGroupInstanced(uint16_t group_idx, uint32_t num_instances) const {
  assert(vb);
  assert(num_vertices);
  assert(num_instances > 0);

  assert(CTechnique::current_active_tech);
  assert(CTechnique::current_active_tech->getVertexDecl() == vertex_decl
    || vertex_decl->isCompatibleWith(CTechnique::current_active_tech->getVertexDecl())
    || fatal("Vertex declaration of mesh %s is %s but the shader %s uses decl %s\n",
      getName().c_str(), vertex_decl->name.c_str(),
      CTechnique::current_active_tech->getName().c_str(), CTechnique::current_active_tech->getVertexDecl()->name.c_str()
    ));

  assert(group_idx >= 0);
  assert(group_idx < groups.size());
  const auto & group = groups[group_idx];

  auto count = group.num_indices;
  auto base = group.first_idx;

  if (ib) {
    Render.ctx->DrawIndexedInstanced(count, num_instances, base, 0, 0);
  }
  else {
    Render.ctx->DrawInstanced(count, num_instances, base, 0);
  }
}

void CMesh::renderRange(uint32_t count, uint32_t base) const {
  assert(vb);
  assert(num_vertices);

  assert(CTechnique::current_active_tech);
  assert(CTechnique::current_active_tech->getVertexDecl() == vertex_decl 
    || vertex_decl->isCompatibleWith( CTechnique::current_active_tech->getVertexDecl() )
    || fatal( "Vertex declaration of mesh %s is %s but the shader %s uses decl %s\n", 
      getName().c_str(), vertex_decl->name.c_str(), 
      CTechnique::current_active_tech->getName().c_str(), CTechnique::current_active_tech->getVertexDecl()->name.c_str()
    ));

  if (ib) {
    Render.ctx->DrawIndexed(count, base, 0);
  }
  else {
    Render.ctx->Draw(count, base);
  }
}

void CMesh::activate() const {
  assert(vb);
  UINT offset = 0;
  Render.ctx->IASetVertexBuffers(0, 1, &vb, &bytes_per_vertice, &offset);
  // We know the values of the DX11 enum and our enums match
  Render.ctx->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)topology);
  activateIndexBuffer();
}

void CMesh::activateIndexBuffer() const {
  if (ib)
    Render.ctx->IASetIndexBuffer(ib, index_fmt, 0);
}

void CMesh::activateAndRender() const {
  activate();
  render();
}

void CMesh::renderGroup(uint16_t group_idx, uint16_t instanced_group_idx) const {
  assert(instanced_group_idx == 0);
  assert(group_idx < groups.size());
  const auto& group = groups[group_idx];
  renderRange(group.num_indices, group.first_idx);
}

void CMesh::setGroups(const VMeshGroups& new_groups) {
  groups = new_groups;
}

void CMesh::renderInMenu() {
  ImGui::LabelText("# Vertices", "%d", num_vertices);
  ImGui::LabelText("# Indices", "%d", num_indices);
  ImGui::LabelText("# Groups", "%ld", groups.size());
  ImGui::LabelText("Bytes Per Vertex", "%d", bytes_per_vertice);
  ImGui::LabelText("Vertex Decl", "%s", vertex_decl ? vertex_decl->name.c_str() : "Null");
  ImGui::DragFloat3("AABB Center", &aabb.Center.x, 0.01f, -25.0f, 25.0f);
  ImGui::DragFloat3("AABB HalfSize", &aabb.Extents.x, 0.01f, 0.0f, 25.0f);
}

// ---------------------------------------------------------
// resource is a vertex buffer (VB) or index buffer (IB)
// if num_bytes_to_update == 0 we will read the resource size
// the max_bytes_allowed is the resource size, which this function does not known
void CMesh::updateResourceFromCPU(ID3D11Resource* resource, const void *new_cpu_data, size_t num_bytes_to_update, size_t max_bytes_allowed) {
  assert(is_dynamic);
  assert(new_cpu_data != nullptr);
  assert(resource);

  // If no bytes are given, update the whole buffer
  if (num_bytes_to_update == 0)
    num_bytes_to_update = max_bytes_allowed;

  // Don't copy beyond the res capacity
  assert(num_bytes_to_update <= max_bytes_allowed);

  D3D11_MAPPED_SUBRESOURCE mapped_resource;

  // Get CPU access to the GPU buffer
  HRESULT hr = Render.ctx->Map(
    resource
    , 0     // Vertex buffers only have one subresource. In case of texture, each mipmap is a subresource
    , D3D11_MAP_WRITE_DISCARD     // 
    , 0
    , &mapped_resource);
  assert(!FAILED(hr));

  // Copy from CPU to GPU
  memcpy(mapped_resource.pData, new_cpu_data, num_bytes_to_update);

  // Close the map
  Render.ctx->Unmap(vb, 0);
}

// --------------------------------------
void CMesh::updateVertsFromCPU(const void *new_cpu_data, size_t num_bytes_to_update) {
  updateResourceFromCPU(vb, new_cpu_data, num_bytes_to_update, num_vertices * vertex_decl->bytes_per_vertex);
}

// --------------------------------------
void CMesh::updateIndicesFromCPU(const void *new_cpu_data, size_t num_bytes_to_update) {
  uint32_t bytes_per_index = index_fmt == DXGI_FORMAT_R16_UINT ? 2 : 4;
  updateResourceFromCPU(ib, new_cpu_data, num_bytes_to_update, num_indices * bytes_per_index);
}

void CMesh::setResourcesName(const std::string& new_name) {
  setDXName(vb, (new_name + ".vb").c_str());
  if( ib )
    setDXName(ib, (new_name + ".ib").c_str());
}
