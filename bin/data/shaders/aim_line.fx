//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

SHADER_CTE_BUFFER(TCtesAimLineScanner, CTE_BUFFER_SLOT_COMP_BUFFERS)
{
  float  length;
  float  dummy1;
  float  dummy2;
  float  dummy3;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos      : SV_POSITION;
  float3 N        : NORMAL;
  float2 Uv       : TEXCOORD0;
  float4 T        : NORMAL1;
  float3 WorldPos : TEXCOORD1;
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

  float i = Pos.z * length;
  output.Pos = Pos;
  output.Pos.z = i;
  output.Pos.xy *= 0.3f;
  output.Pos = mul(output.Pos, World);
  output.WorldPos = output.Pos.xyz;
  output.Pos = mul(output.Pos, ViewProjection);
  output.N = mul(N, (float3x3)World);
  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
  output.Uv = Uv;
  return output;
}



float4 PS(VS_OUTPUT input) : SV_Target
{
  const float3 red = float3(1,0,0);

  float3 dir = normalize(CameraPosition - input.WorldPos);
  dir = normalize(-CameraFront.xyz);
  float3 N = normalize(input.N);
  
  float  NdV = dot(dir, N);
  NdV = pow(1-NdV,4);
  //NdV = clamp(1-NdV, 0.0f, 1.0f);
  //float ramp = txAlbedo.Sample(samLinear, input.Uv + float2(GlobalWorldTime * 0.25,GlobalWorldTime * 0.25)).x;

  return float4(red, NdV);
}