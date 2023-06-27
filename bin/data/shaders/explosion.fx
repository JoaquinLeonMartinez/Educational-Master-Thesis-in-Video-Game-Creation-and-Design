//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos      : SV_POSITION;
  float3 N        : NORMAL;
  float2 Uv       : TEXCOORD0;
  float4 T        : NORMAL1;
};

struct VS_OUTPUT2
{
  float4 Pos : SV_POSITION;
  float2 Uv : TEXCOORD0;
  float4 Color : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT2 VS_QUAD(
  float4 Pos : POSITION,
  float2 Uv: TEXCOORD0,
  float4 Color : COLOR
)
{
  VS_OUTPUT2 output = (VS_OUTPUT2)0;
  output.Pos = mul(Pos, World);
  output.Pos = mul(output.Pos, ViewProjection);
  output.Uv = Uv;
  return output;
}

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


  output.Pos = mul(Pos, World);
  output.Pos.y += sin(GlobalWorldTime) * 0.1;
  output.Pos = mul(output.Pos, ViewProjection);
  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
  output.Uv = Uv;
  return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
  const float4 yellow = float4(1,1,0,1);
  const float4 red = float4(1,0,0,1);

  float ramp = txAlbedo.Sample(samLinear, input.Uv + float2(GlobalWorldTime * 0.25,GlobalWorldTime * 0.25)).x;

  float4 color = lerp(yellow,red, ramp);

  return float4(color);
}

float4 PS_sparks(VS_OUTPUT input) : SV_Target
{
  const float3 white = float3(0.85,0.85,0.85);
  const float3 yellow = float3(1,1,0);
  const  float threshold = 0.35f;

  float ramp = txAlbedo.Sample(samLinear, input.Uv - float2(GlobalWorldTime * 0.5,GlobalWorldTime * 0.5)).x;

  float3 color = lerp(white,yellow, ramp);

  ramp = ramp < threshold;
    if(ramp == 0.0f)
    clip(-1);

  return float4(color, ramp);
}

float4 PS_smoke(VS_OUTPUT input) : SV_Target
{
  const float3 brown = float3(0.015,0.015,0.015);
  const float3 black = float3(0,0,0);
  const  float threshold = 0.35f;

  float ramp = txAlbedo.Sample(samLinear, input.Uv + float2(GlobalWorldTime * 0.4,GlobalWorldTime * 0.4)).x;

  float3 color = lerp(brown,black, ramp);

  ramp = ramp < threshold;
  if(ramp == 0.0f)
    clip(-1);

  return float4(color, ramp);
}

float4 PS_field_smoke(VS_OUTPUT2 input) : SV_Target
{
  const float3 brown = float3(0.015,0.015,0.015);
  const float3 black = float3(0,0,0);
  const float threshold = 0.35f;

  input.Uv -= float2(0.5f,0.5f);
  float a = atan2(input.Uv.y, input.Uv.x) / (2.0f * PI);
  float b = length(input.Uv);
  float2 polar = float2(a,b);

  float ramp = txAlbedo.Sample(samLinear, polar - float2(GlobalWorldTime,GlobalWorldTime)).x;

  float3 color = lerp(brown,black, ramp);

  ramp = ramp < threshold;
  if(ramp == 0.0f)
    clip(-1);

  float l = length(input.Uv);
  float alpha = (l < 0.45f) + (l >= 0.45f) * (1 - ((l - 0.45f) / 0.15f));

  return float4(color,alpha);
}
