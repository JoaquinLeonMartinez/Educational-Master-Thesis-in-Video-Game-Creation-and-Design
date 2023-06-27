#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

typedef Vector2 VEC2;
typedef Vector3 VEC3;
typedef Vector4 VEC4;
typedef Matrix MAT44;
typedef Quaternion QUAT;
typedef DirectX::BoundingBox AABB;
typedef DirectX::BoundingSphere BSphere;

#include "camera.h"
#include "angular.h"
#include "transform.h"
#include "interpolators.h"

extern VEC2 loadVEC2(const json& j, const char* attr, const VEC2& defaultValue = VEC2::Zero);
extern VEC3 loadVEC3(const std::string& str);
extern VEC3 loadVEC3(const json& j, const char* attr);
extern VEC3 loadVEC3(const json& j, const char* attr, const VEC3& defaultValue);
extern VEC4 loadVEC4(const json& j, const char* attr);
extern QUAT loadQUAT(const json& j, const char* attr);
extern VEC4 loadColor(const json& j);
extern VEC4 loadColor(const json& j, const char* attr);
extern VEC4 loadColor(const json& j, const char* attr, const VEC4& defaultValue);

template <typename T>
inline T clamp(T value, T minValue, T maxValue)
{
  return std::max(std::min(value, maxValue), minValue);
}
