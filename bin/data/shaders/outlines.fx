#include "common.fx"

//--------------------------------------------------------------------------------------
float4 PS(
    in float4 iPosition : SV_Position
  , in float2 iTex0 : TEXCOORD0
) : SV_Target
{  
  float edgeSoften = 0.0;

  float edgeN = calculateNormalsEdge(iTex0);
  float edgeD = calculateDistanceEdge(iPosition.xy, iTex0);
  float edge = edgeSoften + (edgeN * edgeD) * (1.0 - edgeSoften);

  float3 color = txAlbedo.Sample(samLinear, iTex0);
  float3 final_color = color - (1 - edge);
  return float4(final_color, 1.0);

  
  //float4 distorsion_r = txGNormal.Sample(samClampLinear, iTex0 + DistortionAmount * distortion.xy * sin(GlobalWorldTime * DistortionSpeed));
}
 

