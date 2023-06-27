#include "mcv_platform.h"
#include "mesh.h"
#include "mesh_io.h"

CMesh* TRawMesh::createRenderMesh() const {
  CMesh* new_mesh = new CMesh();

  // If no AABB is provided, send a null so the CMesh::create computes it.
  const AABB* aabb_ptr = aabb_is_valid ? &aabb : nullptr;

  bool is_ok = new_mesh->create(
    vertices.data(),
    header.num_vertex,
    header.bytes_per_vertex,
    header.vertex_type_name,
    indices.data(),
    header.num_indices,
    header.bytes_per_index,
    (CMesh::eTopology)header.primitive_type,
    false,      // is_dynamic 
    aabb_ptr
  );
  if( !groups.empty() )
    new_mesh->setGroups(groups);
  assert(is_ok);
  return new_mesh;
}

CMesh* loadMeshInstanced(const std::string& name);

class CMeshResourceType : public CResourceType {
public:
  int getNumExtensions() const override { return 2; }
  const char* getExtension(int idx) const override { 
    if (idx == 0)
      return "mesh";
    return "instanced_mesh"; 
  }
  const char* getName() const override {
    return "Meshes";
  }
  IResource* create(const std::string& name) const override {

    CMesh* new_mesh = nullptr;

    if (name.find(".instanced_mesh") != std::string::npos) {
      new_mesh = loadMeshInstanced(name);

    }
    else {
      TRawMesh raw_mesh;
      CFileDataProvider fdp(name.c_str());
      bool is_ok = raw_mesh.load(fdp);
      if (!is_ok)
        return nullptr;
      new_mesh = raw_mesh.createRenderMesh();

      // Fake multi texture mesh
      if (name == "data/meshes/figures_GeoSphere002.mesh") {
        int n = 180 * 3;
        auto groups = raw_mesh.groups;
        groups[0].num_indices -= n;
        groups.resize(groups.size() + 1);
        groups[1].first_idx = groups[0].num_indices;
        groups[1].num_indices = n;
        new_mesh->setGroups(groups);
      }
    }

    assert(new_mesh->isValid());
    new_mesh->setNameAndType(name, this);

    new_mesh->setResourcesName(name);

    return new_mesh;
  }
};

template<>
const CResourceType* getResourceTypeFor<CMesh>() {
  static CMeshResourceType resource_type;
  return &resource_type;
}
