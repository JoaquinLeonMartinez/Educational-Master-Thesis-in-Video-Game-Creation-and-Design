#pragma once

#include "angular.h"

class CCamera {

protected:

  MAT44 view;
  MAT44 projection;
  MAT44 view_projection;

  VEC3  eye;
  VEC3  target;
  VEC3  up_aux;

  VEC3  front;
  VEC3  left;
  VEC3  up;

  float z_near = 0.1f;
  float z_far = 1000.0f;
  float fov_radians = deg2rad( 60.0f );
  float aspect_ratio = 1.0f;
  bool  is_ortho = false;

  // Ortho params
  float ortho_left = 0.f;
  float ortho_top = 0.f;
  float ortho_width = 1.0f;
  float ortho_height = 1.0f;
  bool  ortho_centered = false;

  void updateViewProjection();
  void updateProjection();

  // viewport
  struct TViewport {
    int x0 = 0;
    int y0 = 0;
    int width = 640;
    int height = 480;
  };
  TViewport viewport;

public:

  CCamera();

  MAT44 getView() const { return view; }
  MAT44 getProjection() const { return projection; }
  MAT44 getViewProjection() const { return view_projection; }

  float getNear() const { return z_near; }
  float getFar() const { return z_far; }
  float getFov() const { return fov_radians; }
  float getAspectRatio() const { return aspect_ratio; }

  VEC3  getPosition() const { return eye; }
  VEC3  getTarget() const { return target; }
  VEC3  getFront() const { return front; }
  VEC3  getLeft() const { return left; }
  VEC3  getUp() const { return up; }
  
  void lookAt(VEC3 new_eye, VEC3 new_target, VEC3 new_up_aux = VEC3(0, 1, 0));
  void setProjectionParams(float new_fov_radians, float new_z_near, float new_z_far);
  void setOrthoParams(bool centered, float left, float width, float top, float height, float new_z_near, float new_z_far);
  void setViewport(int x0, int y0, int width, int height);
  bool getScreenCoordsOfWorldCoord(VEC3 world_pos, VEC3 *screen_coords) const;

};
