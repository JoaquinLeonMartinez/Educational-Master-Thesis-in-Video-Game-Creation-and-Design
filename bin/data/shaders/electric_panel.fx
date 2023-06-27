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
  output.Pos = mul(output.Pos, ViewProjection);
  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
  output.Uv = Uv;
  return output;
}

void PS(VS_OUTPUT input
  , out float4 o_deferred : SV_Target0
  , out float4 o_shine : SV_Target1)
{
  const float power = 5.0f;
  const float thickness = 0.2f;
  const float threshold = 2.0f;

  float noise1 = txNoise.Sample(samLinear,input.Uv*1.0f + float2(0.0,GlobalWorldTime*1.7)).x;
  float noise2 = txNoise.Sample(samLinear,input.Uv*0.5f - float2(0.0,GlobalWorldTime*1.7)).y;
  float sum_noise = noise1 + noise2;
  sum_noise = pow(sum_noise,power);

  float noise_clamped = sum_noise > threshold;
  float subs = sum_noise > threshold + thickness;

  float final_noise = noise_clamped - subs;

  float alpha = clamp(final_noise, 0.0,1.0);

  o_deferred = float4(0.0, 0.5, 1.0, alpha);
  o_shine = float4(0.0, 0.5, 1.0, alpha);
}

