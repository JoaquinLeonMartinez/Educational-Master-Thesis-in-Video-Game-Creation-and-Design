#pragma once

#include <variant>

enum class EVariableOperator
{
  Equal = 0,
  NotEqual,
  Greater,
  Less,
  GreaterEqual,
  LessEqual
};

using TVariableValue = std::variant<bool, int, float, std::string, CHandle>;

//struct TVariableValue
//{
//  enum EType { Bool = 0, Int, Float};
//  
//  union
//  {
//    bool bValue;
//    int iValue;
//    float fValue;
//  };
//
//  EType type;
//
//  void set(bool value)
//  {
//    type = Bool;
//    bValue = value;
//  }
//};

struct TVariable
{
  static EVariableOperator parseOperator(const std::string& type);
  static TVariable parseVariable(const json& jData);

  std::string _name;
  TVariableValue _value;

  const std::string& getType() const;
  std::string toString() const;
};
