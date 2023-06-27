#pragma once

#include "resources/resource.h"
#include "render/render.h"

class CCurve : public IResource
{
public:
  void addKnot(const VEC3& pos);
  VEC3 evaluate(float ratio) const;
  void renderDebug(const CTransform& world = CTransform()) const;
  const std::vector<VEC3> getKnots() {
	  return _knots;
  }
  std::vector<VEC3> _knots;
private:
  
};

// -----------------------------------------
// These are the two functions we need to implement to use a new type of curve
template< typename TKnot >
TKnot interpolateCurveKnot(const TKnot* p1, float ratio);

template< typename TKnot >
void drawSegmentBetweenCurveKnots(const TKnot& n1, const TKnot& n2, const MAT44 world, VEC4 color);

template< typename TKnot >
void drawCurveKnot(const TKnot& n1, const MAT44 world, VEC4 color);

// -----------------------------------------
template< typename TNode >
class CKnotsCurve : public IResource
{
  std::vector<TNode> knots;
  bool               show_knots = false;
  int                debug_samples = 100;

public:

  void load(const json& jdata) {
    knots = jdata.get< std::vector< TNode > >();
  }

  void onFileChanged(const std::string& filename) override {
    if (filename == getName()) {
      json j = loadJson(filename);
      load(j);
    }
  }

  void renderInMenu() override {
    ImGui::Checkbox("Show Knots", &show_knots);
    ImGui::DragInt("# Samples", &debug_samples, 0.2f, 2, 512 );
    if (ImGui::TreeNode("Knots")) {
      int idx = 0;
      for (auto& n : knots) {
        char title[64];
        snprintf(title, 64, "Knot %d/%d", idx, (int)knots.size());
        if (ImGui::TreeNode(title)) {
          n.renderInMenu();
          ImGui::TreePop();
        }
        ++idx;
      }
      ImGui::TreePop();
    }
  }

  TNode evaluate(float ratio) const {

    ratio = clamp(ratio, 0.f, 1.f);
    if (ratio >= 1.0f)
      return interpolateCurveKnot(knots.data() + knots.size() - 4, 1.0f);
    int numSections = (int)knots.size() - 3; // -start point, -end point
    int section = static_cast<int>(ratio * numSections);
    float ratioPerSection = 1.f / numSections;
    float sectionRatio = fmodf(ratio, ratioPerSection) / ratioPerSection;
    const int idx = section + 1;
    assert(idx > 0);
    assert(idx + 2 < knots.size() );
    return interpolateCurveKnot(knots.data() + idx - 1, sectionRatio);
  }

  void renderDebug(const MAT44 world = MAT44::Identity, VEC4 color = VEC4(1,1,1,1) ) const {
    CGpuScope gpu_scope(getName().c_str());

    TNode prevNode = evaluate(0.f);
    for (int i = 1; i < debug_samples; ++i) {
      const float ratio = static_cast<float>(i) / static_cast<float>(debug_samples - 1.f);
      TNode newNode = evaluate(ratio);

      drawSegmentBetweenCurveKnots(prevNode, newNode, world, color );

      prevNode = newNode;
    }

    if( show_knots ) {
      CGpuScope gpu_knots(getName().c_str());
      for (auto& n : knots)
        drawCurveKnot(n, world, color);
    }

  }

};

// -----------------------------------------
// Forward declaration that we can read a transform from json, defined elsewhere
void from_json(const json& j, CTransform& t);

// A base curve where each knot is a CTransform
typedef CKnotsCurve< CTransform > CTransCurve;

