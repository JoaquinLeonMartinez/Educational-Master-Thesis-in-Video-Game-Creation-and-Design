#ifndef INC_RENDER_GPU_TRACES_H_
#define INC_RENDER_GPU_TRACES_H_

#include <d3d9.h>         // D3DPERF_*

// Enable profile even in release builds
#define PROFILE         

#if defined(_DEBUG) || defined(PROFILE)

struct CGpuTrace {
  static void push(const char* name) {
    wchar_t wname[128];
    mbstowcs(wname, name, 128);
    D3DPERF_BeginEvent(D3DCOLOR_XRGB(192, 192, 255), wname);
    rmt_BeginD3D11SampleDynamic(name);
  }
  static void pop() {
    rmt_EndD3D11Sample();
    D3DPERF_EndEvent();
  }
  static void setMarker(const char* name, D3DCOLOR clr = D3DCOLOR_XRGB(255, 128, 255)) {
    wchar_t marker_wname[128];
    mbstowcs(marker_wname, name, 128);
    D3DPERF_SetMarker(clr, marker_wname);
  }

};

#else

struct CGpuTrace {
  static void push(const char* name) {}
  static void pop() {}
  static void setMarker(const char* name, D3DCOLOR clr = D3DCOLOR_XRGB(255, 120, 255)) {}
};

#endif

// ------------------------------------
struct CGpuScope {
  CGpuScope(const char *name) {
    CGpuTrace::push(name);
  }
  ~CGpuScope() {
    CGpuTrace::pop();
  }
}; 

#endif

