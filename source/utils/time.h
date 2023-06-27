#pragma once

// ----------------------------------------------------------
// To compute time elapsed between ctor and elapsed()
class CTimer {
  uint64_t time_stamp;
public:
  CTimer() {
    reset();
  }

  // Ticks!
  uint64_t getTimeStamp() const {
    uint64_t now;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&now);
    return now;
  }

  float elapsed() {
    uint64_t now = getTimeStamp();
    uint64_t delta_ticks = now - time_stamp;

    LARGE_INTEGER freq;
    if (::QueryPerformanceFrequency(&freq)) {
      float delta_secs = (float)(delta_ticks) / (float)freq.QuadPart;
      return delta_secs;
    }
    fatal("QueryPerformanceFrequency returned false!!!\n");
    return 0.f;
  }

  // Reset counter to current timestamp
  void reset() {
    time_stamp = getTimeStamp();
  }

  float elapsedAndReset() {
    float delta = elapsed();
    reset();
    return delta;
  }

};

// ----------------------------------------------------------
// Holds a global with the elapsed/unscaled and current time
struct TElapsedTime {
  float  delta = 0.f;
  double current = 0.f;
  float  scale_factor = 1.0f;
  float  delta_unscaled = 0.f;
  float  real_scale_factor = 1.0f;
  int    num_frames = 30;
  int    actual_frame = 0;

  void set(double new_time) {
    delta_unscaled = (float)(new_time - current) * real_scale_factor;
    current = new_time;
    delta = delta_unscaled * scale_factor;
    
    if (num_frames > actual_frame) { 
      #ifndef NDEBUG
            return;
      #endif
      actual_frame++;
      loadedFrame();
    }
  }
  void loadedFrame() {
    delta_unscaled = 0.f;
    delta = 0.f;
  }
};

extern TElapsedTime Time;


