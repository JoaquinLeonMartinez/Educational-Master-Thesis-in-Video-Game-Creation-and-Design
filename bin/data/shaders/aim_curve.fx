//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

SHADER_CTE_BUFFER(TCtesAimCurve, CTE_BUFFER_SLOT_COMP_BUFFERS)
{
  float  height;
  float  v0;
  float  pitch;
  float  length;
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
  float v0x = v0 * cos(-1 * pitch);
  float v0y = v0 * sin(-1 * pitch);
  float y = height + v0y * (i / v0x) - 0.5 * (9.81) * pow((i / v0x), 2);
  output.Pos = Pos;
  output.Pos.z = i;
  output.Pos.y += y;
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
  const float3 white = float3(1,1,1);

  float3 dir = normalize(-CameraFront.xyz);
  float3 N = normalize(input.N);
  float  NdV = saturate(dot(N, dir));
  NdV = clamp(1-NdV, 0.0f, 0.5f);
  //float ramp = txAlbedo.Sample(samLinear, input.Uv + float2(GlobalWorldTime * 0.25,GlobalWorldTime * 0.25)).x;

  return float4(white, NdV);
}
