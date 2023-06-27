
#include "mcv_platform.h"
#include "render.h"
#include "primitives.h"
#include "render/textures/texture.h"

struct SimpleVertex
{
  VEC3 Pos;
  VEC4 Color;
  SimpleVertex() = default;
  SimpleVertex(VEC3 newPos, VEC4 newColor) : Pos(newPos), Color(newColor) {}
};

struct VertexUVColor
{
  VEC3 Pos;
  VEC2 UV;
  VEC4 Color;
  VertexUVColor() = default;
  VertexUVColor(VEC3 newPos, VEC2 newUV, VEC4 newColor) : Pos(newPos), UV(newUV), Color(newColor) {}
};

// cached pointers
const CTechnique*  tech_debug_color = nullptr;
const CTechnique*  tech_debug_pos = nullptr;
const CTechnique*  tech_debug_pos_skin = nullptr;
const CTechnique*  tech_debug_pos_instanced = nullptr;
const CTechnique*  tech_draw_line = nullptr;
const CVertexDeclaration* vdecl_pos_color = nullptr;

// 
CCteBuffer<TCtesCamera> ctes_camera(CTE_BUFFER_SLOT_CAMERAS);
CCteBuffer<TCtesShared> ctes_shared(CTE_BUFFER_SLOT_SHARED);
CCteBuffer<TCtesObject> ctes_object(CTE_BUFFER_SLOT_OBJECT);
CCteBuffer<TCtesDebugLine> ctes_debug_line(CTE_BUFFER_SLOT_DEBUG_LINE);
CCteBuffer<TCtesLight>  ctes_light(CTE_BUFFER_SLOT_LIGHT);
CCteBuffer<TCtesBlur>   ctes_blur(CTE_BUFFER_SLOT_BLUR);
CCteBuffer<TCtesUI>  ctes_ui(CTE_BUFFER_SLOT_UI);
CCteBuffer<TCtesFXAA>   ctes_fxaa(CTE_BUFFER_SLOT_FXAA);
CCteBuffer<TCtesChromaticAberration>   ctes_chr_abr(CTE_BUFFER_SLOT_CHR_ABR);
CCteBuffer<TCtesDamage>   ctes_dam(CTE_BUFFER_SLOT_CHR_ABR);


// -----------------------------------------------------------
bool createAxis(CMesh& mesh) {
  SimpleVertex axis_data[] =
  {
    { VEC3(0.0f,  0.0f, 0.0f), VEC4(1,0,0,1)},
    { VEC3(20.0f,  0.0f, 0.0f), VEC4(1,0,0,1)},
    { VEC3(0.0f,  0.0f, 0.0f), VEC4(0,1,0,1)},
    { VEC3(0.0f,  20.0f, 0.0f), VEC4(0,1,0,1)},
    { VEC3(0.0f,  0.0f, 0.0f), VEC4(0,0,1,1)},
    { VEC3(0.0f,  0.0f, 20.0f), VEC4(0,0,1,1)},
  };
  return mesh.create(axis_data, 6, sizeof(SimpleVertex), "PosColor", nullptr, 0, 0, CMesh::LINE_LIST);
}

// -----------------------------------------------------------
bool createGrid(CMesh& mesh, int samples) {
  std::vector< SimpleVertex > vtxs;
  VEC4 color1(0.2f, 0.2f, 0.2f, 1.0f);
  VEC4 color2(0.3f, 0.3f, 0.3f, 1.0f);
  for (int i = -samples; i <= samples; ++i) {
    VEC4 color = (i % 5) ? color1 : color2;
    SimpleVertex v1 = { VEC3((float)i, 0.0f, (float)-samples), color };
    SimpleVertex v2 = { VEC3((float)i, 0.0f, (float)samples), color };
    vtxs.push_back(v1);
    vtxs.push_back(v2);
    SimpleVertex v3 = { VEC3((float)-samples, 0.0f, (float)i), color };
    SimpleVertex v4 = { VEC3((float)samples, 0.0f, (float)i), color };
    vtxs.push_back(v3);
    vtxs.push_back(v4);
  }
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(SimpleVertex), "PosColor", nullptr, 0, 0, CMesh::LINE_LIST);
}


// -----------------------------------------------------------
bool createCircleXZ(CMesh& mesh, int samples ) {
  std::vector< SimpleVertex > vtxs;
  vtxs.reserve(samples);
  for (int i = 0; i <= samples; ++i) {
    float angle = 2 * (float) M_PI * (float) i / (float) samples;
    SimpleVertex v1 = { yawToVector(angle), VEC4(1,1,1,1) };
    vtxs.push_back(v1);
  }
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(SimpleVertex), "PosColor", nullptr, 0, 0, CMesh::LINE_STRIP);
}

void addFace(std::vector<uint16_t>& idxs, int a, int b, int c, int d) {
  idxs.push_back(a);
  idxs.push_back(c);
  idxs.push_back(b);
  idxs.push_back(a);
  idxs.push_back(d);
  idxs.push_back(c);
}

// -----------------------------------------------------------
bool createCameraFrustumSolid(CMesh& mesh) {
  std::vector<SimpleVertex> vtxs;
  VEC4 clr(1, 1, 1, 1);
  vtxs.emplace_back(VEC3(-1, -1, 0), clr);
  vtxs.emplace_back(VEC3(1, -1, 0), clr);
  vtxs.emplace_back(VEC3(-1, -1, 1), clr);
  vtxs.emplace_back(VEC3(1, -1, 1), clr);
  vtxs.emplace_back(VEC3(-1, 1, 0), clr);
  vtxs.emplace_back(VEC3(1, 1, 0), clr);
  vtxs.emplace_back(VEC3(-1, 1, 1), clr);
  vtxs.emplace_back(VEC3(1, 1, 1), clr);

  std::vector<uint16_t> idxs;
  addFace(idxs, 5, 1, 3, 7);
  addFace(idxs, 1, 0, 2, 3);
  addFace(idxs, 0, 4, 6, 2);
  addFace(idxs, 4, 5, 7, 6);
  addFace(idxs, 0, 1, 5, 4);
  addFace(idxs, 7, 3, 2, 6);

  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(SimpleVertex), "PosColor"
    , idxs.data(), (uint32_t)idxs.size(), sizeof(uint16_t), CMesh::TRIANGLE_LIST);
}

  bool createCameraFrustum(CMesh& mesh) {
  std::vector<SimpleVertex> vtxs;
  VEC4 clr(1, 1, 1, 1);
  vtxs.emplace_back(VEC3(-1, -1, 0), clr);
  vtxs.emplace_back(VEC3(1, -1, 0), clr);
  vtxs.emplace_back(VEC3(-1, -1, 1), clr);
  vtxs.emplace_back(VEC3(1, -1, 1), clr);
  vtxs.emplace_back(VEC3(-1, 1, 0), clr);
  vtxs.emplace_back(VEC3(1, 1, 0), clr);
  vtxs.emplace_back(VEC3(-1, 1, 1), clr);
  vtxs.emplace_back(VEC3(1, 1, 1), clr);

  std::vector<uint16_t> idxs;
  for (int i = 0; i < 4; ++i) {
    // Lines along +x
    idxs.push_back(i * 2);
    idxs.push_back(i * 2 + 1);
    // Vertical lines
    idxs.push_back(i);
    idxs.push_back(i + 4);
  }
  idxs.push_back(0);
  idxs.push_back(2);
  idxs.push_back(1);
  idxs.push_back(3);
  idxs.push_back(4);
  idxs.push_back(6);
  idxs.push_back(5);
  idxs.push_back(7);
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(SimpleVertex), "PosColor"
    , idxs.data(), (uint32_t) idxs.size(), sizeof(uint16_t), CMesh::LINE_LIST);
}

// ----------------------------------
// To render wired AABB's
bool createWiredUnitCube(CMesh& mesh) {
  std::vector<SimpleVertex> vtxs =
  {
    { VEC3(-0.5f,-0.5f, -0.5f),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.5f,-0.5f, -0.5f),   VEC4(1, 1, 1, 1) },
    { VEC3(-0.5f, 0.5f, -0.5f),  VEC4(1, 1, 1, 1) },
    { VEC3(0.5f, 0.5f, -0.5f),   VEC4(1, 1, 1, 1) },    // 
    { VEC3(-0.5f,-0.5f, 0.5f),   VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.5f,-0.5f, 0.5f),    VEC4(1, 1, 1, 1) },
    { VEC3(-0.5f, 0.5f, 0.5f),   VEC4(1, 1, 1, 1) },
    { VEC3(0.5f, 0.5f, 0.5f),    VEC4(1, 1, 1, 1) },    // 
  };
  const std::vector<uint16_t> idxs = {
    0, 1, 2, 3, 4, 5, 6, 7
    , 0, 2, 1, 3, 4, 6, 5, 7
    , 0, 4, 1, 5, 2, 6, 3, 7
  };
  const int nindices = 8 * 3;
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(SimpleVertex), "PosColor"
    , idxs.data(), nindices, sizeof(uint16_t), CMesh::LINE_LIST);
}

// ----------------------------------
bool createPlaneXYTextured(CMesh& mesh) {
  std::vector<VertexUVColor> vtxs =
  {
    { VEC3(-0.5f, -0.5f, 0.0f),  VEC2(0, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.5f, -0.5f, 0.0f),  VEC2(1, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(-0.5f, 0.5f, 0.0f),  VEC2(0, 1),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.5f, 0.5f, 0.0f),  VEC2(1, 1),  VEC4(1, 1, 1, 1) },    // 
  };
  const std::vector<uint16_t> idxs = {
    0, 1, 2, 3
  };
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(VertexUVColor), 
    "PosUvColor"
    , idxs.data(), (uint32_t)idxs.size(), sizeof(uint16_t), CMesh::TRIANGLE_STRIP);
}

// ----------------------------------
bool createPlaneXYTexturedUI(CMesh& mesh) {
  std::vector<VertexUVColor> vtxs =
  {
    { VEC3(0.0f, 0.0f, 0.0f),  VEC2(0, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(1.0f, 0.0f, 0.0f),  VEC2(1, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.0f, 1.0f, 0.0f),  VEC2(0, 1),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(1.0f, 1.0f, 0.0f),  VEC2(1, 1),  VEC4(1, 1, 1, 1) },    // 
  };
  const std::vector<uint16_t> idxs = {
    0, 1, 2, 3
  };
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(VertexUVColor),
    "PosUvColor"
    , idxs.data(), (uint32_t)idxs.size(), sizeof(uint16_t), CMesh::TRIANGLE_STRIP);
}

// ----------------------------------
bool createBoltQuad(CMesh& mesh) {
  std::vector<VertexUVColor> vtxs =
  {
    { VEC3(0.0f, -0.5f, 0.0f),  VEC2(0, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.0f, -0.5f, 1.0f),  VEC2(1, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.0f, 0.5f, 0.0f),  VEC2(0, 1),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(0.0f, 0.5f, 1.0f),  VEC2(1, 1),  VEC4(1, 1, 1, 1) },    // 
  };
  const std::vector<uint16_t> idxs = {
    0, 1, 2, 3
  };
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(VertexUVColor),
    "PosUvColor"
    , idxs.data(), (uint32_t)idxs.size(), sizeof(uint16_t), CMesh::TRIANGLE_STRIP);
}

// ----------------------------------
// Full screen quad to dump textures in screen
bool createUnitQuadXYCentered(CMesh& mesh) {
  const float min = -0.5f;
  const float max = 0.5f;
  std::vector<VertexUVColor> vtxs =
  {
    { VEC3(min, min, 0.0f),  VEC2(0, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(max, min, 0.0f),  VEC2(1, 0),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(min, max, 0.0f),  VEC2(0, 1),  VEC4(1, 1, 1, 1) },    // 
    { VEC3(max, max, 0.0f),  VEC2(1, 1),  VEC4(1, 1, 1, 1) },    // 
  };
  const std::vector<uint16_t> idxs = {
    0, 1, 2, 3
  };
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(VertexUVColor),
    "PosUvColor"
    , idxs.data(), (uint32_t)idxs.size(), sizeof(uint16_t), CMesh::TRIANGLE_STRIP);
}

// ----------------------------------
// Full screen quad to dump textures in screen
bool createUnitQuadXY(CMesh& mesh) {
  const VEC4 white(1, 1, 1, 1);
  const std::vector<SimpleVertex> vtxs = {
    { VEC3(0, 0, 0), white }
  , { VEC3(1, 0, 0), white }
  , { VEC3(0, 1, 0), white }
  , { VEC3(1, 1, 0), white }
  };
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(SimpleVertex),
    "PosColor"
    , nullptr, 0, 0, CMesh::TRIANGLE_STRIP);
}

bool createTrail(CMesh& mesh) {
  const VEC4 white(1, 1, 1, 1);
  const std::vector<SimpleVertex> vtxs = {
    { VEC3(0, 0, 0), white }
  , { VEC3(1, 0, 0), white }
  , { VEC3(0, 1, 0), white }
  , { VEC3(1, 1, 0), white }
  };
  return mesh.create(vtxs.data(), (uint32_t)vtxs.size(), sizeof(SimpleVertex),
    "PosColor"
    , nullptr, 0, 0, CMesh::TRIANGLE_STRIP);
}

// The template wants a name, a funcion, and optional args
// The first arg of the function is a mesh, then the args.
template< typename fn, typename ...Args>
void registerMesh(const char* new_name, fn f, Args... args) {
  PROFILE_FUNCTION(new_name);
  CMesh* mesh = new CMesh();
  // Call the fn, first arg is the mesh
  bool is_ok = f(*mesh, args...);
  // Assign the .mesh to the name
  std::string name = std::string(new_name) + ".mesh";
  // Assign the name and class
  mesh->setNameAndType(name, getResourceTypeFor<CMesh>());
  mesh->setResourcesName(name);
  // Save it in the Resources container
  Resources.registerResource(mesh);
}

bool createRenderPrimitives() {
  bool is_ok = true;
  PROFILE_FUNCTION("createRenderPrimitives");

  registerMesh("axis", createAxis);
  registerMesh("grid", createGrid, 100);
  registerMesh("unit_plane_xy", createPlaneXYTextured);
  registerMesh("unit_plane_xy_UI", createPlaneXYTexturedUI);
  registerMesh("unit_circle_xz", createCircleXZ, 32);
  registerMesh("unit_frustum", createCameraFrustum);
  registerMesh("unit_frustum_solid", createCameraFrustumSolid);
  registerMesh("unit_wired_cube", createWiredUnitCube);
  registerMesh("unit_quad_xy", createUnitQuadXY);
  registerMesh("unit_quad_xy_centered", createUnitQuadXYCentered);
  registerMesh("bolt_quad", createBoltQuad);
  registerMesh("trail_mesh", createTrail);
  
  tech_debug_color = Resources.get("debug_color.tech")->as<CTechnique>();
  tech_debug_pos = Resources.get("debug_pos.tech")->as<CTechnique>();
  tech_debug_pos_skin = Resources.get("debug_pos_skin.tech")->as<CTechnique>();
  tech_debug_pos_instanced = Resources.get("debug_pos_instanced.tech")->as<CTechnique>();
  tech_draw_line = Resources.get("debug_line.tech")->as<CTechnique>();
  vdecl_pos_color = getVertexDeclByName("PosColor");
  is_ok &= ctes_camera.create("Camera");
  is_ok &= ctes_shared.create("Shared");
  is_ok &= ctes_object.create("Object");
  is_ok &= ctes_light.create("Light");
  is_ok &= ctes_debug_line.create("DebugLine");
  is_ok &= ctes_blur.create("Blur");
  is_ok &= ctes_fxaa.create("FXAA");
  is_ok &= ctes_chr_abr.create("ChromAbr");
  is_ok &= ctes_ui.create("UI");
  is_ok &= ctes_dam.create("Damage");
  assert(is_ok);

  ctes_debug_line.activate();     // This could be done once per runtime
  ctes_object.activate();
  ctes_camera.activate();
  ctes_shared.activate();
  ctes_light.activate();
  ctes_blur.activate();
  ctes_ui.activate();
  ctes_fxaa.activate();
  ctes_dam.activate();

  return is_ok;
}

void destroyRenderPrimitives() {
  ctes_debug_line.destroy();     // This could be done once per runtime
  ctes_object.destroy();
  ctes_camera.destroy();
  ctes_shared.destroy();
  ctes_light.destroy();
  ctes_blur.destroy();
  ctes_ui.destroy();
  ctes_fxaa.destroy();
  ctes_chr_abr.destroy();
  ctes_dam.destroy();
}

void drawLine(VEC3 src, VEC3 dst, VEC4 color) {
  ctes_debug_line.DebugSrc = VEC4(src.x, src.y, src.z, 1.0f);
  ctes_debug_line.DebugDst = VEC4(dst.x, dst.y, dst.z, 1.0f);
  ctes_debug_line.DebugColor = color;
  ctes_debug_line.updateGPU();
  tech_draw_line->activate();
  Render.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  Render.ctx->Draw(2, 0);
}

void activateObject(MAT44 world, VEC4 color) {
  ctes_object.ObjColor = color;
  ctes_object.World = world;
  ctes_object.updateGPU();
}

void activateCamera(const CCamera& camera, int width, int height) {
  ctes_camera.Projection = camera.getProjection();
  ctes_camera.View = camera.getView();
  ctes_camera.ViewProjection = camera.getViewProjection();
  ctes_camera.InverseViewProjection = ctes_camera.ViewProjection.Invert();
  ctes_camera.CameraFront = camera.getFront();
  ctes_camera.CameraZFar = camera.getFar();
  ctes_camera.CameraPosition = camera.getPosition();
  ctes_camera.CameraZNear = camera.getNear();

  ctes_camera.CameraTanHalfFov = tanf(camera.getFov() * 0.5f);
  ctes_camera.CameraInvResolution = VEC2(1.0f / (float)width, 1.0f / (float)height);
  ctes_camera.CameraAspectRatio = camera.getAspectRatio();

  ctes_camera.CameraLeft = camera.getLeft();
  ctes_camera.CameraUp = camera.getUp();

  // Simplify conversion from screen coords to world coords 
  MAT44 m = MAT44::CreateScale(-ctes_camera.CameraInvResolution.x * 2.f, -ctes_camera.CameraInvResolution.y * 2.f, 1.f)
    * MAT44::CreateTranslation(1, 1, 0)
    * MAT44::CreateScale(ctes_camera.CameraTanHalfFov * ctes_camera.CameraAspectRatio, ctes_camera.CameraTanHalfFov, 1.f)
    * MAT44::CreateScale(ctes_camera.CameraZFar)
    ;

  // To avoid converting the range -1..1 to 0..1 in the shader
  // we concatenate the view_proj with a matrix to apply this offset
  MAT44 mtx_offset = MAT44::CreateScale(VEC3(0.5f, -0.5f, 1.0f))
                   * MAT44::CreateTranslation(VEC3(0.5f, 0.5f, 0.0f));
  ctes_camera.CameraProjWithOffset = camera.getProjection() * mtx_offset;

  // Now the transform local to world coords part
  // float3 wPos =
  //     CameraFront.xyz * view_dir.z
  //   + CameraLeft.xyz  * view_dir.x
  //   + CameraUp.xyz    * view_dir.y
  MAT44 mtx_axis = MAT44::Identity;
  mtx_axis.Forward(-camera.getFront());      // -getFront() because MAT44.Forward negates our input
  mtx_axis.Left(-camera.getLeft());
  mtx_axis.Up(camera.getUp());

  ctes_camera.CameraScreenToWorld = m * mtx_axis;

  ctes_camera.updateGPU();
}

// ---------------------------------------------
void activateDebugTech(const CMesh* mesh) {
  if (mesh->getVertexDecl() == vdecl_pos_color)
    tech_debug_color->activate();
  else if (mesh->getVertexDecl()->name == "PosNUvTanSkin")
    tech_debug_pos_skin->activate();
  else if (strncmp( mesh->getVertexDecl()->name.c_str(), "Instance", 8 ) == 0 )
    tech_debug_pos_instanced->activate();
  else
    tech_debug_pos->activate();
}

// ---------------------------------------------
void drawMesh(const CMesh* mesh, MAT44 world, VEC4 color) {
  activateObject(world, color);
  activateDebugTech(mesh);
  mesh->activateAndRender();
}

void drawCircle(VEC3 center, float radius, VEC4 color) {
  MAT44 world = MAT44::CreateScale(radius) * MAT44::CreateTranslation(center);
  const CMesh* mesh = Resources.get("unit_circle_xz.mesh")->as<CMesh>();
  drawMesh(mesh, world, color);
}

// ---------------------------------------------
void drawWiredAABB(VEC3 center, VEC3 half, MAT44 world, VEC4 color) {
  // Accede a una mesh que esta centrada en el origen y
  // tiene 0.5 de half size
  //auto mesh = Resources.get("wired_unit_cube.mesh")->as<CRenderMesh>();
  MAT44 unit_cube_to_aabb = MAT44::CreateScale(VEC3(half) * 2.f)
    * MAT44::CreateTranslation(center)
    * world;
  const CMesh* mesh = Resources.get("unit_wired_cube.mesh")->as<CMesh>();
  drawMesh(mesh, unit_cube_to_aabb, color);
}

void drawWiredSphere(MAT44 user_world, float radius, VEC4 color) {
  MAT44 world = MAT44::CreateScale(radius) * user_world;
  const CMesh* mesh = Resources.get("unit_circle_xz.mesh")->as<CMesh>();

  //Draw 3 circleXZ 
  drawMesh(mesh, world, color);
  drawMesh(mesh, MAT44::CreateRotationX((float)M_PI_2) * world, color);
  drawMesh(mesh, MAT44::CreateRotationZ((float)M_PI_2) * world, color);
}

void drawWiredSphere(VEC3 center, float radius, VEC4 color) {
  drawWiredSphere(MAT44::CreateTranslation(center), radius, color);
}

void drawAxis(MAT44 world) {
  const CMesh* mesh = Resources.get("axis.mesh")->as<CMesh>();
  drawMesh(mesh, world, VEC4(1,1,1,1));
}

// ---------------------------------------------
void drawFullScreenQuad(const std::string& tech_name, const CTexture* texture) {
  CGpuScope gpu_scope(tech_name.c_str());
  auto* tech = Resources.get(tech_name)->as<CTechnique>();
  assert(tech);
  tech->activate();
  if (texture)
    texture->activate(TS_ALBEDO);
  auto* mesh = Resources.get("unit_quad_xy.mesh")->as<CMesh>();
  mesh->activateAndRender();
}
