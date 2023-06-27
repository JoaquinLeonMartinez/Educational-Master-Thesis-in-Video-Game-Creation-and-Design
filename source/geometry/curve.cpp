#include "mcv_platform.h"
#include "curve.h"

class CCurveResourceType : public CResourceType {
public:
  const char* getExtension( int idx ) const override { return "curve"; }
  const char* getName() const override
  {
    return "Curves";
  }
  IResource* create(const std::string& name) const override
  {
    CCurve* new_curve = new CCurve();
    
    auto jData = loadJson(name);
    for (auto& jKnot : jData)
    {
      VEC3 pos = loadVEC3(jKnot);
      new_curve->addKnot(pos);
    }

    new_curve->setNameAndType(name, this);
    return new_curve;
  }
};

template<>
const CResourceType* getResourceTypeFor<CCurve>() {
  static CCurveResourceType resource_type;
  return &resource_type;
}

void CCurve::addKnot(const VEC3& pos)
{
  _knots.push_back(pos);
}

VEC3 CCurve::evaluate(float ratio) const
{
  ratio = clamp(ratio, 0.f, 1.f);
  int numSections = (int)_knots.size() - 3; // -start point, -end point
  int section = ratio >= 1.f ? numSections - 1 : static_cast<int>(ratio * numSections);
  float ratioPerSection = 1.f / numSections;
  float sectionRatio = fmodf(ratio, ratioPerSection) / ratioPerSection;

  const int idx = section + 1;

  const VEC3 p1 = _knots[idx - 1];
  const VEC3 p2 = _knots[idx];
  const VEC3 p3 = _knots[idx + 1];
  const VEC3 p4 = _knots[idx + 2];

  return VEC3::CatmullRom(p1, p2, p3, p4, sectionRatio);
}

void CCurve::renderDebug(const CTransform& world) const
{
  const int nSamples = 100;
  const VEC4 color(1.f, 1.f, 0.f, 1.f);
  const MAT44 mWorld = world.asMatrix();
  VEC3 prevPos = VEC3::Transform(evaluate(0.f), mWorld);

  for (int i = 1; i < nSamples; ++i)
  {
    const float ratio = static_cast<float>(i) / static_cast<float>(nSamples);
    const VEC3 pos = VEC3::Transform(evaluate(ratio), mWorld);

    drawLine(prevPos, pos, color);

    prevPos = pos;
  }
}

// ------------------------------------------------
template<>
CTransform interpolateCurveKnot<CTransform>(const CTransform* p, float ratio) {
 
  VEC3 newPos = VEC3::CatmullRom(
    p[0].getPosition(),
    p[1].getPosition(),
    p[2].getPosition(),
    p[3].getPosition(),
    ratio);

  QUAT newQuat = QUAT::Slerp(p[1].getRotation(), p[2].getRotation(), ratio);

  return CTransform(newPos, newQuat, 1.0f);
}

template<>
void drawSegmentBetweenCurveKnots< CTransform >(const CTransform& n1, const CTransform& n2, const MAT44 world, VEC4 color) {
  VEC3 p1 = VEC3::Transform(n1.getPosition(), world);
  VEC3 p2 = VEC3::Transform(n2.getPosition(), world);
  drawLine(p1, p2, color);
}

template<>
void drawCurveKnot< CTransform >(const CTransform& n, const MAT44 world, VEC4 color) {
  MAT44 final_world = n.asMatrix() * world;
  drawAxis(final_world);
}

class CTransCurveResourceType : public CResourceType {
public:
  const char* getExtension(int idx) const override { return "trans_curve"; }
  const char* getName() const override
  {
    return "TransCurve";
  }
  IResource* create(const std::string& name) const override
  {
    CTransCurve* new_curve = new CTransCurve();
    new_curve->setNameAndType(name, this);
    new_curve->onFileChanged(name);
    return new_curve;
  }
};

template<>
const CResourceType* getResourceTypeFor<CTransCurve>() {
  static CTransCurveResourceType resource_type;
  return &resource_type;
}
