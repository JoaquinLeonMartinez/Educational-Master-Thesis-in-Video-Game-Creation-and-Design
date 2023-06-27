#include "common.fx"

//--------------------------------------------------------------------------------------
float4 PS( 
  float4 iPosition : SV_POSITION,
  float2 iUV       : TEXCOORD0
  ) : SV_Target
{
  const float3 red = float3(1,0,0);
  int3 ss_load_coords = uint3(iPosition.xy, 0);

  float dist = distance(iUV.xy, float2(0.5f,0.5f));
  float dither = txNormal.Sample(samLinear,iUV * 5).xyz * dist;
  dither = pow(dither,1.25f) * 3.0f * grade;

  float3 color = txAlbedo.Sample(samLinear, iUV).xyz;
  float3 final_color = lerp(color,red,dither);

  return float4(final_color,1);
}
