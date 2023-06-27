#include "common.fx"

//--------------------------------------------------------------------------------------
float4 PS(
    in float4 iPosition : SV_Position
  , in float2 iTex0 : TEXCOORD0
) : SV_Target
{  
  float2 direction = float2(sin(GlobalWorldTime), cos(GlobalWorldTime));
  float3 distortion = txNoise.Sample( samLinear, iTex0 + direction * WaterEffectSpeed);
  
  float4 distorsion_r = txAlbedo.Sample(samClampLinear, iTex0 + DistortionAmount * 0.005 * distortion.xy * sin(GlobalWorldTime * DistortionSpeed)); 
  float4 distorsion_g = txAlbedo.Sample(samClampLinear, iTex0 - DistortionAmount * 0.005 * distortion.xy * sin(GlobalWorldTime * DistortionSpeed));
  float4 distorsion_b = txAlbedo.Sample(samClampLinear, iTex0 + DistortionAmount * 0.005 * distortion.xy * sin(GlobalWorldTime * DistortionSpeed));
  float4 distorsion_a = txAlbedo.Sample(samClampLinear, iTex0 - DistortionAmount * 0.005 * distortion.xy * sin(GlobalWorldTime * DistortionSpeed));

  float alpha = (distorsion_r.a+distorsion_g.a+distorsion_b.a) / 3;
  return float4(distorsion_r.r, distorsion_g.g, distorsion_b.b, alpha);
}
 

float4 PS2(
    in float4 iPosition : SV_Position
  , in float2 iTex0 : TEXCOORD0
) : SV_Target
{  
  const float d = DistortionAmount * 0.01f;
  float colR = txAlbedo.Sample(samClampLinear, float2(iTex0.x - d, iTex0.y -d)).x;
  float colG = txAlbedo.Sample(samClampLinear, iTex0).y;
  float colB = txAlbedo.Sample(samClampLinear, float2(iTex0.x + d, iTex0.y + d)).z;

  return float4(colR, colG, colB, 1);
}