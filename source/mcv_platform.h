#pragma once

// Global settings
#define _CRT_SECURE_NO_WARNINGS     // Don't warn about using fopen..
#define WIN32_LEAN_AND_MEAN         // Reduce the size of things included in windows.h
#define _USE_MATH_DEFINES           // M_PI M_PI_2

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

// C/C++
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstdarg>
#include <cstdint>        // uint32_t
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <map>

// Windows/OS Platform
#define NOMINMAX                    // To be able to use std::min without windows problems
#include <windows.h>
#include <d3d11.h>

#include "profiling/Remotery.h"
#include "imgui/imgui.h"

#include "utils/json.hpp"
using json = nlohmann::json;

#include "geometry/geometry.h"
#include "profiling/profiling.h"
#include "utils/utils.h"
#include "resources/resource.h"
#include "handle/handle.h"
#include "render/render.h"

