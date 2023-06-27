#include "mcv_platform.h"
#include "camera.h"

CCamera::CCamera() {
  projection = MAT44::CreatePerspectiveFieldOfView(
    fov_radians, aspect_ratio, z_near, z_far
  );
  lookAt(VEC3(1, 1, 1), VEC3(0, 0, 0));
}

void CCamera::updateViewProjection() {
  view_projection = view * projection;
}

void CCamera::updateProjection() {
  projection = MAT44::CreatePerspectiveFieldOfView(
    fov_radians, aspect_ratio, z_near, z_far
  );
  updateViewProjection();
}

void CCamera::setProjectionParams(float new_fov_radians, float new_z_near, float new_z_far) {
  is_ortho = false;
  fov_radians = new_fov_radians;
  z_near = new_z_near;
  z_far = new_z_far;
  updateProjection();
}

void CCamera::setOrthoParams(bool is_centered, float new_left, float new_width, float new_top, float new_height, float new_z_near, float new_z_far) {
  is_ortho = true;
  ortho_centered = is_centered;
  ortho_width = new_width;
  ortho_height = new_height;
  ortho_left = new_left;
  ortho_top = new_top;
  
  aspect_ratio = fabsf(ortho_width / ortho_height);
  z_far = new_z_far;
  z_near = new_z_near;

  if( is_centered )
    projection = MAT44::CreateOrthographic( ortho_width, ortho_height, z_near, z_far);
  else
    projection = MAT44::CreateOrthographicOffCenter(ortho_left, ortho_width, ortho_height, ortho_top, z_near, z_far);

  viewport.x0 = 0;
  viewport.y0 = 0;
  viewport.width = (int)ortho_width;
  viewport.height = (int)ortho_height;
  aspect_ratio = (float)ortho_width / (float)ortho_height;

  // Position and target and unset!
  eye = VEC3(ortho_width * 0.5f, ortho_height * 0.5f, z_near);
  front.x = 0;
  front.y = 0;
  front.z = 1;
  left.x = 1;
  left.y = 0;
  left.z = 0;
  up.x = 0;
  up.y = 1;
  up.z = 0;
  view = MAT44::Identity;
  updateViewProjection();
}

void CCamera::lookAt(VEC3 new_eye, VEC3 new_target, VEC3 new_up_aux) {

  view = MAT44::CreateLookAt(new_eye, new_target, new_up_aux);

  eye = new_eye;
  target = new_target;
  up_aux = new_up_aux;

  // Regenerate 3 main axis
  front = (target - eye);
  front.Normalize();
  left = new_up_aux.Cross(front);
  left.Normalize();
  up = front.Cross(left);

  updateViewProjection();
}

void CCamera::setViewport(int x0, int y0, int width, int height) {
  // save params
  viewport.x0 = x0;
  viewport.y0 = y0;
  viewport.width = width;
  viewport.height = height;

  aspect_ratio = (float)width / (float)height;

  if (!is_ortho)
    setProjectionParams(fov_radians, z_near, z_far);
}

bool CCamera::getScreenCoordsOfWorldCoord(VEC3 world_pos, VEC3* result) const {

  // It's also dividing by w  -> [-1..1]
  VEC3 pos_in_homo_space = VEC3::Transform(world_pos, getViewProjection());

  // Convert to 0..1 and then to viewport coordinates
  VEC3 pos_in_screen_space(
    viewport.x0 + (pos_in_homo_space.x + 1.0f) * 0.5f * viewport.width,
    viewport.y0 + (1.0f - pos_in_homo_space.y) * 0.5f * viewport.height,
    pos_in_homo_space.z
  );

  assert(result);
  *result = pos_in_screen_space;

  // Return true if the coord is inside the frustum
  return pos_in_homo_space.x >= -1.0f && pos_in_homo_space.x <= 1.0f
    && pos_in_homo_space.y >= -1.0f && pos_in_homo_space.y <= 1.0f
    && pos_in_homo_space.z >= 0.0f && pos_in_homo_space.z <= 1.0f
    ;
}
