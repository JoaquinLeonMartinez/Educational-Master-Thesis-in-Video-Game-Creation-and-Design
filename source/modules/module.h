#pragma once
#include "mcv_platform.h"

class IModule
{
  std::string _name;
  bool _active = false;

  //IModule(const IModule& other) = delete; //this line makes the module scripting to fail

public:

/*
IModule() = default;
IModule(const IModule&) = default;
*/


  IModule(const std::string& name)
  {
    _name = name;
  }

  virtual bool start() { return true; }
  virtual void stop() { } 
  const std::string& getName() const
  {
    return _name;
  }

  virtual void update(float dt) {}
  virtual void renderDebug() {}
  virtual void renderInMenu() {}

  bool isActive() const
  {
    return _active;
  }

  void setActive( bool active) {
    _active = active;
  }
};

using VModules = std::vector<IModule*>;
