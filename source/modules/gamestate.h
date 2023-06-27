#pragma once
#include "mcv_platform.h"
#include "module.h"

class CGamestate : public VModules
{
public:
  std::string _name;
};

using VGamestates = std::vector<CGamestate>;
