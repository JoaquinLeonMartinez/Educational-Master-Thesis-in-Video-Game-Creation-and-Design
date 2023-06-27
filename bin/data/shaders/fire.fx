//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

SHADER_CTE_BUFFER(TCtesFire, CTE_BUFFER_SLOT_COMP_BUFFERS)
{
  float  randomNumb;
  float3 fire_dummy3;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
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
VS_OUTPUT VS(
  float4 Pos : POSITION,
  float3 N : NORMAL,
  float2 Uv: TEXCOORD0,
  float4 T : NORMAL1
)
{
  VS_OUTPUT output = (VS_OUTPUT)0;
  float4 auxPos = Pos;
  if(Pos.y > -0.01){
    float r = 0.3;
    float off = randomNumb * 2.3;
    auxPos.x += r * cos(GlobalWorldTime * 3 + off);
    auxPos.z += r * sin(GlobalWorldTime * 2 + off);
    auxPos.y += r * sin(GlobalWorldTime * 1 + off);
  }
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
float4 PS(VS_OUTPUT input) : SV_Target
{
  float4 red = float4(1,0,0,1);
  float4 yellow = float4(1,1,0,1);
  float4 orange = float4(1,0.2f,0,1);
  float4 blue = float4(0,0,1,1);
  float4 green = float4(0,1,0,1);

  //for color
  const float _offset = 0.0f;
  //for rim
  const float _edge = 0.7f;
  const float _hard = 5.0f;
  //for shape
  const float _height = 0.70f;
  //for distortion
  const float _scrollX = 0.0f;
  const float _scrollY = 0.4f;
  const float _distort = 0.25f;
  //for visuals
  const float _threshold = 0.8f;
  const float _rim = 0.25f;
  const float _scale = 1.3f;

  float4 distortion = txRoughness.Sample(samLinear, input.Uv + randomNumb) * _distort;
  float4 voronoi_noise = txNormal.Sample(samLinear, (float2((input.Uv.x - GlobalWorldTime * _scrollX) + distortion.g  ,(input.Uv.y + GlobalWorldTime * _scrollY) + distortion.r) * float2(_scale,_scale))+ randomNumb) ;
  voronoi_noise.a = voronoi_noise.z;
  float shapetex = lerp(-0.5,1, input.Uv.y);
  
  voronoi_noise += shapetex * _threshold * 0.9f;
  float4 final_voronoi = voronoi_noise;

  float flame = final_voronoi.x > _threshold;
  float4 flamecolored = flame * yellow;

  float4 flamerim = (final_voronoi.x > _threshold - _rim) - flame;
  float4 flamecolored2 = flamerim * orange;
  float4 finalcolor = flamecolored + flamecolored2;
  return finalcolor;
}


float4 PS_2(VS_OUTPUT input) : SV_Target
{
  float4 red = float4(1,0,0,1);
  float4 yellow = float4(1,1,0,1);
  float4 orange = float4(1,0.2f,0,1);
  float4 blue = float4(0,0,1,1);
  float4 green = float4(0,1,0,1);

  //for color
  const float _offset = 0.0f;
  //for rim
  const float _edge = 0.7f;
  const float _hard = 5.0f;
  //for shape
  const float _height = 0.70f;
  //for distortion
  const float _scrollX = 0.0f;
  const float _scrollY = 1.12f + randomNumb * 0.2;
  const float _distort = 0.25f;
  //for visuals
  const float _threshold = 0.3f;
  const float _rim = 0.1f;
  const float _scale = 1.3f;

  float4 distortion = txRoughness.Sample(samLinear, input.Uv + randomNumb) * _distort;
  float4 voronoi_noise = txNormal.Sample(samLinear, (float2((input.Uv.x - GlobalWorldTime * _scrollX) + distortion.g  ,(input.Uv.y + GlobalWorldTime * _scrollY) + distortion.r) * float2(_scale,_scale))+ randomNumb) ;
  voronoi_noise.a = voronoi_noise.z;
  float shapetex = txAlbedo.Sample(samLinear,input.Uv).x;
  float shapetex2 = lerp(0.5,1.8, input.Uv.y);
  
  voronoi_noise *= shapetex * shapetex2;
  float4 final_voronoi = voronoi_noise;

  float flame = final_voronoi.x > _threshold;
  float4 flamecolored = flame * yellow;

  float4 flamerim = (final_voronoi.x > _threshold - _rim) - flame;
  float4 flamecolored2 = flamerim * orange;
  float4 finalcolor = flamecolored + flamecolored2;
  return finalcolor;
}

float4 PS_aux(VS_OUTPUT input) : SV_Target
{
  const float lineAmplitude = 1.0f;
  const float lineLength = 20.0f;
  const float lineSpeed = 1.0f;

  float2 uv = float2(input.Uv.x * lineLength, (input.Uv.y * lineAmplitude) + (GlobalWorldTime * lineSpeed));
  float2 uv2 = uv + float2( 1, 1) + float2(sin(GlobalWorldTime), cos(GlobalWorldTime));

  float3 tex1 = txNormal.Sample(samLinear, uv); //use this for the color ramp
  float3 tex2 = txMetallic.Sample(samLinear, uv);
  float3 tex3 = txMetallic.Sample(samLinear, uv2);

  float3 tex4 = (tex2 * tex3);

  return float4(tex2, 1);
}