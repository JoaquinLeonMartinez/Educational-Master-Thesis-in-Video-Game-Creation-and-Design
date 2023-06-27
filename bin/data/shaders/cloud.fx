//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"
#include "pbr.inc"
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

VS_OUTPUT VS(
  VS_INPUT input
)
{
  // Use world from the constants uniform
  VS_OUTPUT output = (VS_OUTPUT)0;
  float noise = txNormal.SampleLevel(samLinear,input.Uv + GlobalWorldTime * 0.5,0, 0).x;
  output.Pos = mul(input.Pos, World);
  output.Pos.xyz += 0.5f * noise * input.N.xyz * (sin(GlobalWorldTime)  * 0.25f * noise + 1.5f);
  output.WorldPos = output.Pos.xyz;
  output.Pos = mul(output.Pos, ViewProjection);
  output.N = mul(input.N, (float3x3)World);
  output.T = float4( mul(input.T.xyz, (float3x3)World), input.T.w);
  output.Uv = input.Uv;
  return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
  return float4(1,1,1,0.5f);
}
