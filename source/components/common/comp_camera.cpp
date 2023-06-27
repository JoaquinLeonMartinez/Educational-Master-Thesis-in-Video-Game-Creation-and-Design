#include "mcv_platform.h"
#include "comp_camera.h"
#include "comp_transform.h"
DECL_OBJ_MANAGER("camera", TCompCamera);

void TCompCamera::debugInMenu() {
  float fov_deg = rad2deg(getFov());
  float new_znear = getNear();
  float new_zfar = getFar();

  bool changed = ImGui::Checkbox("Is Ortho", &is_ortho);

  if (is_ortho) {
    bool keep_aspect_ratio = true;
    ImGui::Checkbox("Keep Ratio", &keep_aspect_ratio);
    changed |= ImGui::DragFloat("Width", &ortho_width, 0.1f, 0.1f, 512.0f);
    if (keep_aspect_ratio && changed) {
      ortho_height = ortho_width / aspect_ratio;
      ImGui::Text("Height: %f", ortho_height);
    }
    else
      changed |= ImGui::DragFloat("Height", &ortho_height, 0.1f, 0.1f, 512.0f);
    changed |= ImGui::Checkbox("Centered", &ortho_centered);
    if (!ortho_centered) {
      changed |= ImGui::DragFloat("Left", &ortho_left, 0.1f, 0.1f, 512.0f);
      changed |= ImGui::DragFloat("Top", &ortho_top, 0.1f, 0.1f, 512.0f);
    }
  }
  else
    changed = ImGui::DragFloat("Fov", &fov_deg, 0.1f, 30.f, 175.f);

  changed |= ImGui::DragFloat("Z Near", &new_znear, 0.001f, 0.01f, 5.0f);
  changed |= ImGui::DragFloat("Z Far", &new_zfar, 1.0f, 2.0f, 3000.0f);

  if (changed && new_znear > 0.f && new_znear < new_zfar) {
    if (is_ortho)
      setOrthoParams(ortho_centered, ortho_left, ortho_width, ortho_top, ortho_height, new_znear, new_zfar);
    else
      setProjectionParams(deg2rad(fov_deg), new_znear, new_zfar);
  }

  VEC3 f = getFront();
  ImGui::LabelText("Front", "%f %f %f", f.x, f.y, f.z);
  VEC3 up = getUp();
  ImGui::LabelText("Up", "%f %f %f", up.x, up.y, up.z);
  VEC3 left = getLeft();
  ImGui::LabelText("Left", "%f %f %f", left.x, left.y, left.z);

  ImGui::LabelText("ViewPort", "@(%d,%d) %dx%d", viewport.x0, viewport.y0, viewport.width, viewport.height);
  ImGui::LabelText("Aspect Ratio", "%f", aspect_ratio);
}

void TCompCamera::load(const json& j, TEntityParseContext& ctx) {
  float znear = j.value("near", getNear());
  float zfar = j.value("far", getFar());
  if (j.value("ortho", false)) {
    float new_width = j.value("ortho_width", ortho_width);
    float new_height = j.value("ortho_height", ortho_height);
    float new_left = j.value("ortho_left", ortho_left);
    float new_top = j.value("ortho_top", ortho_top);
    bool centered = j.value("ortho_centered", ortho_centered);
    setOrthoParams(centered, new_left, new_width, new_top, new_height, znear, zfar);
  }
  else {
    float fov = deg2rad(j.value("fov", rad2deg(getFov())));
    setProjectionParams(fov, znear, zfar);
  }

}

void TCompCamera::renderDebug() {
  CGpuScope gpu_scope("ViewVolume");
  MAT44 inv_view_proj = getViewProjection().Invert();

  // xy between -1..1 and z betwee 0 and 1
  auto mesh = Resources.get("unit_frustum.mesh")->as<CMesh>();

  //// Sample several times to 'view' the z distribution along the 3d space
  const int nsamples = 10;
  for (int i = 1; i < nsamples; ++i) {
    float f = (float)i / (float)(nsamples - 1);
    MAT44 world = MAT44::CreateScale(1.f, 1.f, f) * inv_view_proj;
    drawMesh(mesh, world, VEC4(1, 1, 1, 1));
  }

}

void TCompCamera::update(float delta) {
  // The camera follows the entity
  PROFILE_FUNCTION("ComponentCamera");
  TCompTransform* c = get<TCompTransform>();
  this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());
}

