//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

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
  const float power = 5.0f;
  const float thickness = 0.5f;
  const float threshold = 0.7f;

  float2 uv = input.Uv - float2(0.5f,0.5f);
  float a = atan2(uv.y, uv.x) / (2.0f * PI);
  float b = length(uv);
  float2 polar = float2(a,b);

  const float2x2 rot_matrix = { cos(GlobalWorldTime *0.5f), -sin(GlobalWorldTime*0.5f),
    sin(GlobalWorldTime*0.5f), cos(GlobalWorldTime*0.5f)
  };
  float2 rot_uv = mul((polar) - float2(0.5f,0.5f), rot_matrix);

  float noise1 = txNoise.Sample(samLinear,rot_uv*2.5f + float2(0.0,GlobalWorldTime*3.7)).x;
  float noise2 = txNoise.Sample(samLinear,rot_uv*2.85f - float2(0.0,GlobalWorldTime*1.7)).y;
  float sum_noise = noise1 + noise2;
  sum_noise = pow(sum_noise,power);

  float noise_clamped = sum_noise > threshold;
  float subs = sum_noise > threshold + thickness;

  float final_noise = noise_clamped - subs;

  float dist = clamp(length(input.Uv - float2(0.5f,0.5f)),0.0,0.5);

  dist = 1 - (dist > 0.4f ? ((dist - 0.4f)/0.1f) : 0.0f);

  float alpha = clamp(final_noise * dist, 0.0,1.0);

  o_deferred = float4(0.0, 0.5, 1.0, alpha);
  o_shine = float4(0.0, 0.5, 1.0, alpha);
}
