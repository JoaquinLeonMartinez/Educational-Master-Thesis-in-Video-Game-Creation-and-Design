#include "mcv_platform.h"
#include "mesh_instanced.h"

CMesh* loadMeshInstanced(const std::string& name) {
  dbg("Creating instanced mesh %s\n", name.c_str());
  auto mesh = new CMeshInstanced();
  json j = loadJson(name);
  mesh->configure(j);
  return mesh;
}

// -----------------------------------------------------------------
void CMeshInstanced::configure(const json& j) {
  instanced_mesh = Resources.get(j["instanced_mesh"])->as<CMesh>();
  std::string decl = j.value("instances_type", "");
  vertex_decl = getVertexDeclByName(decl);
  assert(vertex_decl);
  reserveGPUInstances(j.value("num_instances_reserved", 4));
}

// -----------------------------------------------------------------
void CMeshInstanced::reserveGPUInstances(size_t new_max_instances) {

  // Allocate only if we require more than what we actually have allocated
  if (new_max_instances > num_instances_allocated_in_gpu) {

    assert(vertex_decl);

    // Do we have an initial value?, start from 4
    if (!num_instances_allocated_in_gpu)
      num_instances_allocated_in_gpu = 4;

    // Keep increasing in powers of 2
    while (new_max_instances > num_instances_allocated_in_gpu)
      num_instances_allocated_in_gpu *= 2;

    // We are about to change the ib/vb, so free the old version
    if (isValid())
      destroy();

    dbg("Reallocating %ld GPU instances for mesh %s\n", num_instances_allocated_in_gpu, getName().c_str());

    // Create the VB as a dynamic buffer to hold a maximum of N instances
    bool is_ok = create(
      nullptr,                  // no vertex data provided, we might be just allocating
      (uint32_t) num_instances_allocated_in_gpu, 
      vertex_decl->bytes_per_vertex,  // Total bytes required
      vertex_decl->name,
      nullptr, 0, 0,            // No index data
      CMesh::POINT_LIST,
      true                      // is dynamic
    );


    assert(is_ok && isValid());
    
    setResourcesName( getName() );
  }
}

// -----------------------------------------------------------------
// Update the GPU with the new data
void CMeshInstanced::setInstancesData(
  const void* data
, size_t total_instances
, size_t bytes_per_instance
) {
  assert(bytes_per_instance > 0);
  assert(bytes_per_instance == getVertexDecl()->bytes_per_vertex);
  reserveGPUInstances( total_instances );
  if( data )
    updateVertsFromCPU(data, total_instances * bytes_per_instance);
  // If there is only one group, we can set the size
  if( groups.size() == 1)
    setGroupSize(0, (uint32_t)total_instances);
}

// --------------------------------------
void CMeshInstanced::setGroupSize(uint32_t group_idx, uint32_t new_size_for_group) {
  assert(group_idx < (uint32_t)groups.size());
  // Ensure we are not programmed to render more indices/vertexs than our size
  assert(ib ? (new_size_for_group <= num_indices) : (new_size_for_group <= num_vertices));
  groups[group_idx].num_indices = new_size_for_group;
}

// -----------------------------------------------------------------
// Configure the two streams and send the mesh to render
void CMeshInstanced::renderGroup(uint16_t group_idx, uint16_t instances_group_idx) const {

  assert(isValid());
  assert(instanced_mesh);
  assert(instanced_mesh->isValid());
  assert(instances_group_idx < groups.size());

  auto& sb = groups[instances_group_idx];
  if (sb.num_indices == 0)
    return;

  // Set the source of both streams
  ID3D11Buffer* vbs[2] = {
    instanced_mesh->vb,      // 0 -> Instanced mesh
    vb                       // 1 -> Instances data
  };

  // Saltos entre vertices para cada stream
  UINT strides[2] = {
    instanced_mesh->getVertexDecl()->bytes_per_vertex,
    vertex_decl->bytes_per_vertex
  };

  // Send vertexs from the start of each vertex buffer
  UINT offsets[2] = {
    0,
    0
  };
  Render.ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);

  // Set primitive topology
  Render.ctx->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)instanced_mesh->topology);

  assert(group_idx < instanced_mesh->groups.size());

  auto& instanced_group = instanced_mesh->groups[group_idx];
  assert(instanced_group.num_indices > 0);

  // Is Indexed?
  if (instanced_mesh->ib) {
    instanced_mesh->activateIndexBuffer();

    // instances_group_idx
    Render.ctx->DrawIndexedInstanced(
      instanced_group.num_indices, 
      sb.num_indices, 
      instanced_group.first_idx, 
      0,
      sb.first_idx
    );
  }
  else {
    Render.ctx->DrawInstanced(
      instanced_group.num_indices, 
      sb.num_indices, 
      0, 
      sb.first_idx
    );
  }

}