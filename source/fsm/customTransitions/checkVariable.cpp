#include "mcv_platform.h"
#include "fsm/customTransitions/checkVariable.h"
#include "fsm/context.h"

void CCheckVariable::load(const json& jData)
{
  _operator = TVariable::parseOperator(jData["operator"]);
  _variable = TVariable::parseVariable(jData["variable"]);
}

bool CCheckVariable::check(const CFSMContext& ctx) const
{
  const TVariable* ctxVariable = ctx.getVariable(_variable._name);
  if (!ctxVariable)
  {
    return false;
  }

  switch (_operator)
  {
    case EVariableOperator::Equal:
    {
      return ctxVariable->_value == _variable._value;
    }
    case EVariableOperator::NotEqual:
    {
      return ctxVariable->_value != _variable._value;
    }
    case EVariableOperator::Greater:
    {
      return ctxVariable->_value > _variable._value;
    }
    case EVariableOperator::GreaterEqual:
    {
      return ctxVariable->_value >= _variable._value;
    }
    case EVariableOperator::Less:
    {
      return ctxVariable->_value < _variable._value;
    }
    case EVariableOperator::LessEqual:
    {
      return ctxVariable->_value <= _variable._value;
    }
  }
  
  return false;
}
