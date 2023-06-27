#include "mcv_platform.h"
#include "collision_mesh.h"

class CCollisionMeshResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "col_mesh"; }
  const char* getName() const override {
    return "Collision Meshes";
  }
  IResource* create(const std::string& name) const override {
    CFileDataProvider fdp(name.c_str());
    CCollisionMesh* new_mesh = new CCollisionMesh();
    bool is_ok = new_mesh->load(fdp);
    assert(is_ok);
    new_mesh->setNameAndType(name, this);
    return new_mesh;
  }
};

template<>
const CResourceType* getResourceTypeFor<CCollisionMesh>() {
  static CCollisionMeshResourceType resource_type;
  return &resource_type;
}
