#pragma once

#include "components/common/comp_base.h"
#include "components/common/comp_name.h"
#include "entity/entity.h"
#include "geometry/curve.h"


class TCompParabolicLaunch : public TCompBase
{
  DECL_SIBLING_ACCESS();

public:
  void load(const json& j, TEntityParseContext& ctx);//meter en jason la velocidad de lanzamiento
  void debugInMenu();
  void renderDebug();
  void calculateParabolicPoints();
  void update(float dt);
  void loadValuesParabola(float new_h, float pitch,float yaw);
  CCurve* curve_dynamic = nullptr; //guardamos vector de puntos de punto maximo y alcance

private:
	float heightPj = 1.80f;
	int nsamples =100;
	float v0 = 20.f; //velocidad en modulo
  float iterations = 100.0f;
	MAT44 _curveTransform;
  VEC4 constants = VEC4::Zero;	

  float h;
  float pitch;
  float yaw;
  float v0x; //componente z
  float v0y; //componente y
  CHandle h_player;
	

};

