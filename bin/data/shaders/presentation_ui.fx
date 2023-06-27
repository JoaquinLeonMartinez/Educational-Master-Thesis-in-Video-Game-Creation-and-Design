//--------------------------------------------------------------------------------------
#include "common.fx"
#include "gbuffer.inc"


// -------------------------------------------------
// What we actually present to the final backbuffer.
float4 PS( 
  float4 iPosition : SV_POSITION,
  float2 iUV       : TEXCOORD0
  ) : SV_Target
{

  int3 ss_load_coords = uint3(iPosition.xy, 0);

  float4 ui = txAlbedo.Sample(samLinear, iUV);

  return ui;
}
