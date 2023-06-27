//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

SHADER_CTE_BUFFER(TCtesBolt, CTE_BUFFER_SLOT_COMP_BUFFERS)
{
  float  lineBoltScale;
  float3 bolt_dummy3;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos : SV_POSITION;
  float2 Uv : TEXCOORD0;
  float4 Color : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(
  float4 Pos : POSITION,
  float2 Uv: TEXCOORD0,
  float4 Color : COLOR
)
{
  VS_OUTPUT output = (VS_OUTPUT)0;
  //Pos.xyz *= sin( WorldTime );
  output.Pos = mul(Pos, World);

  output.Pos = mul(output.Pos, ViewProjection);
  output.Color = Color * ObjColor;
  output.Uv = Uv;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
void PS(VS_OUTPUT input
  , out float4 o_deferred : SV_Target0
  , out float4 o_shine : SV_Target1)
{
  float lineSize = 0.02f / lineBoltScale;

  float noise1 = txNoise.Sample(samLinear,float2(input.Uv.x + GlobalWorldTime * 2,0.5)).x;
  float noise2 = txNoise.Sample(samLinear,float2(input.Uv.x,GlobalWorldTime * 2)).y; //touch this value to change the frecuency
  noise1 = noise1 * 2.0f  - 1.0f;
  noise2 = noise2 * 2.0f  - 1.0f;
  noise1 *= 0.3f * noise2; //touch this value to make it wider
  
  float minvalue = 0.5f - lineSize + noise1;
  float maxvalue = 0.5f + lineSize + noise1;
 
  float uv = (input.Uv.y > minvalue) * (input.Uv.y < maxvalue);

  float3 texture_color = float4(0.0, 0.5, 1.0,1.0) * uv;

  o_deferred = float4(texture_color, uv);
  o_shine = float4(texture_color, uv);
}

struct VS_OUTPUT2
{
  float4 Pos      : SV_POSITION;
  float3 N        : NORMAL;
  float2 Uv       : TEXCOORD0;
  float3 WorldPos : TEXCOORD1;
  float4 T        : NORMAL1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT2 VS_bolt_sphere(
  float4 Pos : POSITION,
  float3 N : NORMAL,
  float2 Uv: TEXCOORD0,
  float4 T : NORMAL1
)
{
  VS_OUTPUT2 output = (VS_OUTPUT2)0;
  float4 auxPos = Pos;
  output.Pos = mul(auxPos, World);
  output.WorldPos = output.Pos.xyz;
  output.Pos = mul(output.Pos, ViewProjection);
  output.N = mul(N, (float3x3)World);
  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
  output.Uv = Uv;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
void PS_bolt_sphere(VS_OUTPUT2 input
  , out float4 o_deferred : SV_Target0
  , out float4 o_shine : SV_Target1)
{ 
  float3 dir = normalize(CameraPosition.xyz - input.WorldPos);
  float3 N = normalize(input.N);
  float  NdV = saturate(dot(N, dir));

  float3 white = float3(1,1,1);
  float variance = sin(GlobalWorldTime* 2) / 6.666f + 1.30f;
  float3 color = lerp(ObjColor.xyz, white, NdV) * variance;

  o_deferred = float4(0.0, 0.5, 1.0, 1.0);
  o_shine = float4(1.0, 1.0, 1.0, 1.0);
}
