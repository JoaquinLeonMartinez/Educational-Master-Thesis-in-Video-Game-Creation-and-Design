#ifndef INC_MODULE_MULTITHREADING_H_
#define INC_MODULE_MULTITHREADING_H_

#include "module.h"

class MultithreadingModule : public IModule {
 
public:
  MultithreadingModule(const std::string& aname) : IModule(aname) { }
  bool start() override;
  void stop() override;
  void update(float dt) override;
  
};

#endif

