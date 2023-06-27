#include "mcv_platform.h"
#include "geometry.h"

// if yaw == 0 => VEC(0,0,1)
// if yaw == 90o => VEC(1,0,0)

VEC3  yawToVector(float yaw) {
  return VEC3(sinf(yaw), 0.0f, cosf(yaw));
}

float vectorToYaw(VEC3 front) {
  return atan2f(front.x, front.z);
}

VEC3  yawPitchToVector(float yaw, float pitch) {
  return VEC3(
      sinf(yaw) * cosf(-pitch)
    ,             sinf(-pitch)
    , cosf(yaw) * cosf(-pitch)
  );
}

void  vectorToYawPitch(VEC3 front, float* yaw, float* pitch) {
  *yaw = vectorToYaw(front);
  // Projection of front in the plane XZ
  float mod_xz = sqrtf(front.x*front.x + front.z*front.z);
  *pitch = -atan2f(front.y, mod_xz);
}

