#pragma once

#include "ui/ui_widget.h"

namespace UI
{
  class CButton : public CWidget
  {
  
  private:
    struct TState
    {
      TImageParams imageParams;
      TTextParams textParams;
    };
    std::map<std::string, TState> _states;
    TState* _currentState = nullptr;
  public:
	  void render() override;
	  void setCurrentState(const std::string& stateName);
	  TState* getCurrentState() { return _currentState; }
	  
    friend class CParser;
  };

}
