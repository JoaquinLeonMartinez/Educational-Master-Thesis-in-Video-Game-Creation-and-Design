
//--------------------------------------------------------------------------------------
// Standard vertex for objects (without skinning)
struct VS_INPUT
{
  float4 Pos : POSITION;
  float3 N : NORMAL;
  float2 Uv: TEXCOORD0;
  float4 T : NORMAL1;
};

struct VS_INPUT_MORPH
{
  float4 Pos : POSITION0;
  float3 N   : NORMAL0;
  float2 Uv  : TEXCOORD0;

  float4 Pos1 : POSITION1;
  float3 N1   : NORMAL1;

  float4 T : NORMAL2;
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
// This is used by normal rendering, skin rendering and instancing rendering
VS_OUTPUT runObjVS(VS_INPUT input, float4x4 world ) {
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(input.Pos, world);
  output.WorldPos = output.Pos.xyz;
  output.Pos = mul(output.Pos, ViewProjection);
  output.N = mul(input.N, (float3x3)world);
  output.T = float4( mul(input.T.xyz, (float3x3)world), input.T.w);
  output.Uv = input.Uv;
  return output;
}
