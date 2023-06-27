#include "mcv_platform.h"
#include "module_multithreading.h"
#include "tbb/tbb.h"

bool MultithreadingModule::start() {
  int nDefThreads = tbb::task_scheduler_init::default_num_threads();
  tbb::task_scheduler_init init(nDefThreads);
  return true;
}

void MultithreadingModule::stop() {
  //todo: stop the scheduler
}

void MultithreadingModule::update(float dt) {
}
