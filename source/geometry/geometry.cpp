#include "mcv_platform.h"

VEC2 loadVEC2(const json& j, const char* attr, const VEC2& defaultValue)
{
  VEC2 v = defaultValue;
  if (j.count(attr) == 0)
  {
    return v;
  }
  auto k = j.value(attr, "0 0");
  sscanf(k.c_str(), "%f %f", &v.x, &v.y);
  return v;
}

VEC3 loadVEC3(const std::string& str) {
  VEC3 v;
  sscanf(str.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
  return v;
}

VEC4 loadVEC4(const json& j, const char* attr) {
  auto k = j.value(attr, "0 0 0 0");
  VEC4 v;
  int n = sscanf(k.c_str(), "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
  assert(n == 4);
  return v;
}

VEC3 loadVEC3(const json& j, const char* attr) {
  auto k = j.value(attr, "0 0 0");
  VEC3 v;
  sscanf(k.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
  return v;
}

VEC3 loadVEC3(const json& j, const char* attr, const VEC3& defaultValue) {
  if (j.count(attr) <= 0)
  {
    return defaultValue;
  }
  return loadVEC3(j, attr);
}

QUAT loadQUAT(const json& j, const char* attr) {
  auto k = j.value(attr, "0 0 0 1");
  QUAT v;
  sscanf(k.c_str(), "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
  return v;
}

VEC4 loadColor(const json& j) {
  auto k = j.get<std::string>();
  VEC4 v;
  sscanf(k.c_str(), "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
  return v;
}

VEC4 loadColor(const json& j, const char* attr) {
  auto k = j.value(attr, "1 1 1 1");
  VEC4 v;
  sscanf(k.c_str(), "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
  return v;
}

VEC4 loadColor(const json& j, const char* attr, const VEC4& defaultValue) {
  if (j.count(attr) <= 0)
  {
    return defaultValue;
  }
  return loadColor(j, attr);
}