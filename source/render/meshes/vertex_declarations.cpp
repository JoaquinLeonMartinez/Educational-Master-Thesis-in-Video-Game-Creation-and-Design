#include "mcv_platform.h"
#include "vertex_declarations.h"

#define MAKE_VERTEX_DECLARATION(name, nbytes) \
  new CVertexDeclaration{ \
    #name, nbytes, layout_##name, ARRAYSIZE(layout_##name) \
  }

// Define the input layout
static D3D11_INPUT_ELEMENT_DESC layout_Pos[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static D3D11_INPUT_ELEMENT_DESC layout_PosColor[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static D3D11_INPUT_ELEMENT_DESC layout_PosUvColor[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static D3D11_INPUT_ELEMENT_DESC layout_PosNUv[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static D3D11_INPUT_ELEMENT_DESC layout_PosNUvT[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static D3D11_INPUT_ELEMENT_DESC layout_PosNUvPosNT[] =
{
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static D3D11_INPUT_ELEMENT_DESC layout_PosNUvTanSkin[] = {
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "BONES",    0, DXGI_FORMAT_R8G8B8A8_UINT,      0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "WEIGHTS",  0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static D3D11_INPUT_ELEMENT_DESC layout_Instance[] = {
  { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world0
  { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world1
  { "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world2
  { "TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world3
};

static D3D11_INPUT_ELEMENT_DESC layout_InstanceColor[] = {
  { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world0
{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world1
{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world2
{ "TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world3
{ "TEXCOORD", 6, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // color
};

static D3D11_INPUT_ELEMENT_DESC layout_InstanceColorTimed[] = {
  { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world0
{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world1
{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world2
{ "TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // world3
{ "TEXCOORD", 6, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // color
{ "TEXCOORD", 7, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // time + 3 dummy
};

static D3D11_INPUT_ELEMENT_DESC layout_ParticleRenderData[] = {
  { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // pos
  { "TEXCOORD", 3, DXGI_FORMAT_R32_FLOAT,          0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // time
  { "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },    // color  // Not used anymore
};

static std::vector< CVertexDeclaration* > all_vertex_decl = {
  MAKE_VERTEX_DECLARATION(Pos, 12),
  MAKE_VERTEX_DECLARATION(PosColor, 28),
  MAKE_VERTEX_DECLARATION(PosUvColor, 36),
  MAKE_VERTEX_DECLARATION(PosNUv, 32),
  MAKE_VERTEX_DECLARATION(PosNUvTanSkin, 56),
  MAKE_VERTEX_DECLARATION(PosNUvT, 48),
  MAKE_VERTEX_DECLARATION(PosNUvPosNT, 72),
  MAKE_VERTEX_DECLARATION(Instance, 64),
  MAKE_VERTEX_DECLARATION(InstanceColor, 80),
  MAKE_VERTEX_DECLARATION(InstanceColorTimed, 80+16),
  MAKE_VERTEX_DECLARATION(ParticleRenderData, 32),
};

const CVertexDeclaration* createInstancedVertexDecl(const std::string& name, size_t idx) {
  // Check if it's an instance vertex declaration
  std::string first = name.substr(0, idx);
  std::string second = name.substr(idx + 3);

  auto a = getVertexDeclByName(first);      // instanced_decl
  auto b = getVertexDeclByName(second);     // instances_decl
  assert(a && b);
  auto num_elems = a->numElements + b->numElements;
  // Create a new layout concatenating both layouts and:
  //  -- LAYOUT A AS NORMAL    --
  //  -- LAYOUT B AS INSTANCED --
  D3D11_INPUT_ELEMENT_DESC* layout = new D3D11_INPUT_ELEMENT_DESC[num_elems];
  memcpy(layout, a->layout, a->numElements * sizeof(D3D11_INPUT_ELEMENT_DESC));

  auto layout_b = layout + a->numElements;
  memcpy(layout_b, b->layout, b->numElements * sizeof(D3D11_INPUT_ELEMENT_DESC));

  // Change the layout B to be 'instanced data' with rate = 1
  for (uint32_t i = 0; i < b->numElements; ++i) {
    layout_b[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;
    layout_b[i].InstanceDataStepRate = 1;
    layout_b[i].InputSlot = 1;    // Comes from stream 1
  }

  uint32_t nbytes = a->bytes_per_vertex + b->bytes_per_vertex;
  auto decl = new CVertexDeclaration{
    name, nbytes, layout, num_elems
  };
  decl->instancing = true;

  // Register it
  all_vertex_decl.push_back(decl);

  return decl;
}

const CVertexDeclaration* getVertexDeclByName(const std::string& name) {
  for (auto it : all_vertex_decl) {
    if (it->name == name)
      return it;
  }

  // Check if it's an instanced mesh vertex decl
  auto idx = name.find("_x_");
  if (idx != std::string::npos)
    return createInstancedVertexDecl( name, idx );

  return nullptr;
}

// If this == PosNUv and other is Pos, yes PosNUv is compatible with Pos
bool CVertexDeclaration::isCompatibleWith(const CVertexDeclaration* other) const {
  return name.substr(0, other->name.length()) == other->name;
}
