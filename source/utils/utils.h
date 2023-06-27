#pragma once

#include "data_provider.h"

// -----------------------------------------
#include "murmur3/murmur3.h"
uint32_t getID(const char* txt);
uint32_t getID(const void* buff, size_t nbytes);

bool isPressed(int key);

json loadJson(const std::string& filename);

void dbg(const char* fmt, ...);
bool fatal(const char* fmt, ...);

bool fileExists(const char* filename);

float unitRandom();
float randomFloat(float vmin, float vmax);
VEC2 getMouseInWindow();
bool pointInRectangle(VEC2 point, VEC2 rectangle_size, VEC2 rectangle_position);
template <typename T>
inline T Lerp(T a, T b, T percent) {
  return (a + percent * (b - a));
}
template <typename T> 
int sign(T val) {
  return (T(0) < val) - (val < T(0));
}

//http://www.rorydriscoll.com/2016/03/07/frame-rate-independent-damping-using-lerp/
inline float Damp(float a, float b, float lambda, float dt) {
  return Lerp(a, b, 1.f - exp(-lambda * dt));
}

inline VEC2 Damp(VEC2 a, VEC2 b, float lambda, float dt) {
  return VEC2::Lerp(a, b, 1.f - exp(-lambda * dt));
}

inline VEC3 Damp(VEC3 a, VEC3 b, float lambda, float dt) {
  return VEC3::Lerp(a, b, 1.f - exp(-lambda * dt));
}

inline VEC4 Damp(VEC4 a, VEC4 b, float lambda, float dt) {
  return VEC4::Lerp(a, b, 1.f - exp(-lambda * dt));
}

inline QUAT Damp(QUAT a, QUAT b, float lambda, float dt) {
  return QUAT::Lerp(a, b, 1.f - exp(-lambda * dt));
}

#include "named_values.h"
#include "utils/time.h"
#include "utils/file_context.h"
