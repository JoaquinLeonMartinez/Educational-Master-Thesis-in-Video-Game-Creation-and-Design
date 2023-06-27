#include "common.fx"

//--------------------------------------------------------------------------------------
float4 PS( 
  float4 iPosition : SV_POSITION,
  float2 iUV       : TEXCOORD0
  ) : SV_Target
{
  float3 color = txAlbedo.Sample(samLinear, iUV).xyz;
  iUV.y = 1 - iUV.y;


  float2 coord = (iUV.xy - float2(0.5,0.5)) * (CameraInvResolution.y/CameraInvResolution.x) * 2.0;

  float rf = sqrt(dot(coord,coord)) * 0.45f;
  float rf2_1 = rf * rf + 1.0;
  float e = 1.0 / (rf2_1 * rf2_1);

  float3 final_color = color * e;

  return float4(final_color,1);
}
