#include "mcv_platform.h"
#include "ai_controller.h"

void IAIController::update(float dt)
{
	
	
	if (!_pausedAI) {
		PROFILE_FUNCTION("IAIController");
		assert(!state.empty());
		assert(statemap.find(state) != statemap.end());
		// this is a trusted jump as we've tested for coherence in ChangeState
		(this->*statemap[state])(Time.delta);
	}
	
	
}

bool IAIController::isPaused() {
	return _pausedAI;
}
void IAIController::setPaused(bool _pausedAI) {
	this->_pausedAI = _pausedAI;
}

void IAIController::ChangeState(const std::string& newstate)
{
  // try to find a state with the suitable name
  if (statemap.find(newstate) == statemap.end())
  {
    // the state we wish to jump to does not exist. we abort
    fatal("Invalid ChangeState(%s)\n", newstate.c_str());
  }

  state = newstate;
  
}


void IAIController::AddState(const std::string& name, statehandler sh)
{
  // try to find a state with the suitable name
  if (statemap.find(name) != statemap.end())
  {
    // the state we wish to jump to does exist. we abort
    fatal("Invalid AddState(%s). Already defined.\n", name.c_str());
  }
  statemap[name] = sh;
}

void IAIController::onMsgAIPaused(const TMsgAIPaused & msg)
{
    _pausedAI = msg.isPaused;
}

