#ifndef _AICONTROLLER_H
#define _AICONTROLLER_H

// ai controllers using maps to function pointers for easy access and scalability. 

// we put this here so you can assert from all controllers without including everywhere
#include <assert.h>	
#include <string>
#include <map>
#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"

// states are a map to member function pointers, to 
// be defined on a derived class. 

class IAIController;
//Un statehandler es un puntero a funcion de IAIController
typedef void (IAIController::*statehandler)(float);

class IAIController : public TCompBase {
public:

protected:

  std::string                         state;
  // the states, as maps to functions
  std::map<std::string, statehandler> statemap;

public:
  void ChangeState(const std::string&);	// state we wish to go to
  virtual void Init() { }// resets the controller
  void update(float dt);	// recompute behaviour
  void AddState(const std::string&, statehandler);
  const std::string getState() const { return state; }
  bool _pausedAI = false;
  bool isPaused();
  void setPaused(bool _pausedAI);
  void onMsgAIPaused(const TMsgAIPaused & msg);
};

#endif