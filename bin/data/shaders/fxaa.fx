#include "common.fx"

#define FXAA_PC 1
#define FXAA_HLSL_4 1
#define FXAA_QUALITY__PRESET 39
#define FXAA_GREEN_AS_LUMA 1

#include "Fxaa3_11.h"

//--------------------------------------------------------------------------------------
// This shader is expected to be used only with the mesh unitQuadXY.mesh
// Where the iPos goes from 0,0..1,1
void VS(
    in float4 iPos : POSITION
  , out float4 oPos : SV_POSITION
  , out float2 oTex0 : TEXCOORD0
)
{
  // Passthrough of coords and UV's
  oPos = float4(iPos.x * 2 - 1., 1 - iPos.y * 2, 0, 1);
  oTex0 = iPos.xy;
}


//--------------------------------------------------------------------------------------
float4 PS(
    in float4 iPosition : SV_Position
  , in float2 iTex0 : TEXCOORD0
) : SV_Target
{
  //ONE VERSION
  FxaaTex inputFXAATex = { samClampBiLinear, txAlbedo };
  float pixelWidth = (1.0f / screenWidth);
  float pixelHeight = (1.0f / screenHeight);

  float2 pixelCenter = float2(iTex0.x - pixelWidth, iTex0.y - pixelHeight);

  return FxaaPixelShader(
    pixelCenter,                             // FxaaFloat2 pos,
    FxaaFloat4(0,0,0,0),                     // FxaaFloat4 fxaaConsolePosPos,
    inputFXAATex,                            // FxaaTex tex,
    inputFXAATex,                            // FxaaTex fxaaConsole360TexExpBiasNegOne,
    inputFXAATex,                            // FxaaTex fxaaConsole360TexExpBiasNegTwo,
    float2(pixelWidth,pixelHeight),          // FxaaFloat2 fxaaQualityRcpFrame,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),      // FxaaFloat4 fxaaConsoleRcpFrameOpt,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),      // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),      // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
    sub_pix,                                 // FxaaFloat fxaaQualitySubpix,
    edge_threshold,                          // FxaaFloat fxaaQualityEdgeThreshold,
    edge_threshold_min,                      // FxaaFloat fxaaQualityEdgeThresholdMin,
    0.f,                                     // FxaaFloat fxaaConsoleEdgeSharpness,
    0.f,                                     // FxaaFloat fxaaConsoleEdgeThreshold,
    0.f,                                     // FxaaFloat fxaaConsoleEdgeThresholdMin,
    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)       // FxaaFloat fxaaConsole360ConstDir,
  );

}
  