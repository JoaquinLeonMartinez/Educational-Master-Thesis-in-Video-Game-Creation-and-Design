//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

SHADER_CTE_BUFFER(TCtesBoom, CTE_BUFFER_SLOT_COMP_BUFFERS)
{
  float  alpha;
  float3 dummy;
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
  float2 uv = input.Uv;
  uv.x *= -1;
  uv.y *= -1;
  float4 tex = txAlbedo.Sample(samLinear,uv);

  if(tex.a < 0.7f)
    clip(-1);

  //-------------------
  //comic shading
  const float2x2 rot_matrix = { cos(brush_rotation), -sin(brush_rotation),
    sin(brush_rotation), cos(brush_rotation)
  };
  uv *= 3.0f;
  float2 rot_uv = mul(uv - float2(0.5f,0.5f), rot_matrix);
  float brush = saturate(1.0f - saturate(txNoise.Sample(samLinear,rot_uv).x));
  float4 dots = brush * tex;
  //end comic shading
  //-------------------
  tex.xyz -= dots.xyz;
  tex.a = alpha;

  o_deferred = tex;
}
