#pragma once

#include "meshes/mesh.h"
#include "render/shaders/shader.h"
#include "render/shaders/cte_buffer.h"
#include "../bin/data/shaders/constants.h"

bool createRenderPrimitives();
void destroyRenderPrimitives();

extern CCteBuffer<TCtesCamera> ctes_camera;
extern CCteBuffer<TCtesObject> ctes_object;
extern CCteBuffer<TCtesShared> ctes_shared;
extern CCteBuffer<TCtesLight>  ctes_light;
extern CCteBuffer<TCtesBlur>   ctes_blur;
extern CCteBuffer<TCtesUI>  ctes_ui;
extern CCteBuffer<TCtesFXAA>   ctes_fxaa;
extern CCteBuffer<TCtesChromaticAberration>   ctes_chr_abr;
extern CCteBuffer<TCtesDamage>   ctes_dam;

void activateObject(MAT44 world, VEC4 color = VEC4(1, 1, 1, 1));
void activateCamera(const CCamera& camera, int width, int height);
void activateDebugTech(const CMesh* mesh);

void drawMesh(const CMesh* mesh, MAT44 world, VEC4 color);
void drawCircle(VEC3 center, float radius, VEC4 color);
void drawAxis(MAT44 world);
void drawLine(VEC3 src, VEC3 dst, VEC4 color);
void drawWiredSphere(VEC3 center, float radius, VEC4 color);
void drawWiredSphere(MAT44 world, float radius, VEC4 color);
void drawWiredAABB(VEC3 center, VEC3 half, MAT44 world, VEC4 color);
void drawFullScreenQuad(const std::string& tech_name, const CTexture* texture);

