//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

SHADER_CTE_BUFFER(TCtesSmokeDust, CTE_BUFFER_SLOT_COMP_BUFFERS)
{
  float  alpha_erosion_entrance;
  float  alpha_erosion_exit;
  float2 dummy;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos      : SV_POSITION;
  float3 N        : NORMAL;
  float2 Uv       : TEXCOORD0;
  float4 WorldPos : TEXCOORD1;
  float4 T        : NORMAL1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(
  float4 Pos : POSITION,
  float3 N : NORMAL,
  float2 Uv: TEXCOORD0,
  float4 T : NORMAL1
)
{
  VS_OUTPUT output = (VS_OUTPUT)0;
  float4 auxPos = Pos;
  float noise = txNormal.SampleLevel(samLinear,Uv + GlobalWorldTime * 0.1f,0, 0).x;
  float factor = 0.5f * alpha_erosion_exit;
  //auxPos.xyz += N * noise * factor;

  output.WorldPos = mul(auxPos, World);

  output.Pos = mul(output.WorldPos, ViewProjection);
  output.N = mul(N, (float3x3)World);
  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
  output.Uv = Uv;
  return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
  float noise1 = 1 - txNormal.SampleLevel(samLinear, input.Uv, 0, 0).x;
  float noise2 = txAlbedo.SampleLevel(samLinear, input.Uv, 0, 0).x;
  float alpha = (noise1 < alpha_erosion_entrance) * (noise2 > alpha_erosion_exit);

  return float4(ObjColor.xyz,alpha);
}