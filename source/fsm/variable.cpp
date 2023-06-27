#include "mcv_platform.h"
#include "fsm/variable.h"
#include "entity/entity.h"

EVariableOperator TVariable::parseOperator(const std::string& type)
{
  static const std::map<std::string, EVariableOperator> operatorNames = {
    { "equal", EVariableOperator::Equal },
    { "notEqual", EVariableOperator::NotEqual },
    { "greater", EVariableOperator::Greater },
    { "greaterEqual", EVariableOperator::GreaterEqual },
    { "less", EVariableOperator::Less },
    { "lessEqual", EVariableOperator::LessEqual }
  };

  const auto& it = operatorNames.find(type);
  return it != operatorNames.end() ? it->second : EVariableOperator::Equal;
}

TVariable TVariable::parseVariable(const json& jData)
{
  TVariable var;
  
  var._name = jData.value<std::string>("name", "");

  const std::string& varType = jData.value<std::string>("type", "boolean");
  if (varType == "boolean")
  {
    var._value = jData.value<bool>("value", false);
  }
  else if (varType == "int")
  {
    var._value = jData.value<int>("value", 0);
  }
  else if (varType == "float")
  {
    var._value = jData.value<float>("value", 0.f);
  }
  else if (varType == "string")
  {
    var._value = jData.value<std::string>("value", "");
  }
  else if (varType == "handle")
  {
    var._value = CHandle();
  }

  return var;
}

const std::string& TVariable::getType() const
{
  static std::string kBool("boolean");
  static std::string kInt("int");
  static std::string kFloat("float");
  static std::string kString("string");
  static std::string kHandle("handle");
  static std::string kUndefined("undefined");

  if (std::holds_alternative<bool>(_value))             return kBool;
  else if (std::holds_alternative<int>(_value))         return kInt;
  else if (std::holds_alternative<float>(_value))       return kFloat;
  else if (std::holds_alternative<std::string>(_value)) return kString;
  else if (std::holds_alternative<CHandle>(_value))     return kHandle;
  else                                                  return kUndefined;
}

std::string TVariable::toString() const
{
  if (std::holds_alternative<bool>(_value))             return std::to_string(std::get<bool>(_value));
  else if (std::holds_alternative<int>(_value))         return std::to_string(std::get<int>(_value));
  else if (std::holds_alternative<float>(_value))       return std::to_string(std::get<float>(_value));
  else if (std::holds_alternative<std::string>(_value)) return std::get<std::string>(_value);
  else if (std::holds_alternative<CHandle>(_value))
  {
    CEntity* e = std::get<CHandle>(_value);
    return e ? e->getName() : "invalid_entity";
  }
  else                                                  return "undefined_value";
}
