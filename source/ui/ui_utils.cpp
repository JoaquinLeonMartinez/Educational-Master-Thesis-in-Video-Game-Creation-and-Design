#pragma once

#include "mcv_platform.h"
#include "ui/ui_utils.h"
#include "render/textures/texture.h"

namespace UI
{
  void renderBitmap(const MAT44& world, const CTexture* texture, const VEC2& minUV, const VEC2& maxUV, const VEC4& color, bool additive)
  {
    MAT44 adjust = MAT44::CreateScale(2.f / Render.width, 1.f / Render.height, 1.f);
    activateObject(world * adjust);
    
    ctes_ui.UIminUV = minUV;
    ctes_ui.UImaxUV = maxUV;
    ctes_ui.UItint = color;
    ctes_ui.updateGPU();

    const std::string& combinative_tech = "ui.tech";
    const std::string& additive_tech = "ui_additive.tech";
    const std::string& tech_name = additive ? additive_tech : combinative_tech;

    auto* tech = Resources.get(tech_name)->as<CTechnique>();
    assert(tech);
    tech->activate();
    if (texture)
      texture->activate(TS_ALBEDO);
    auto* mesh = Resources.get("unit_plane_xy_UI.mesh")->as<CMesh>();
    mesh->activateAndRender();
  }

  void renderText(const MAT44& transform, const CTexture* texture, const std::string& text, const VEC2& size)
  {
    // assuming texture has the characters in ASCII order
    constexpr char firstCh = ' ';
    constexpr int numRows = 8;
    constexpr int numCols = 8;
    constexpr VEC2 cellSize(1.f / numCols, 1.f / numRows);

    MAT44 world = transform;
    MAT44 tr = MAT44::CreateTranslation(size.x, 0.f, 0.f);

    for (const char& ch : text)
    {
      const char cellCh = ch - firstCh;
      const int row = cellCh / numCols;
      const int col = cellCh % numCols;
      const VEC2 minUV(static_cast<float>(col) / static_cast<float>(numCols), static_cast<float>(row) / static_cast<float>(numRows));
      const VEC2 maxUV = minUV + cellSize;

      renderBitmap(world, texture, minUV, maxUV);

      world = world * tr;
    }
  }
}
