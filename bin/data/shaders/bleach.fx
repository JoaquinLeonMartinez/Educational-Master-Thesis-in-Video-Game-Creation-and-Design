//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"
#include "pbr.inc"

SHADER_CTE_BUFFER(TCtesBleach, CTE_BUFFER_SLOT_COMP_BUFFERS)
{
  float  size = 1.0f;
  float3 dummy3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// This is used by normal rendering, skin rendering and instancing rendering
VS_OUTPUT VS(
  VS_INPUT input
)
{
  // Use world from the constants uniform
  VS_OUTPUT output = (VS_OUTPUT)0;
  output.Pos = mul(input.Pos * float4(size,1,size,1), World);
  output.WorldPos = output.Pos.xyz;
  output.Pos = mul(output.Pos, ViewProjection);
  output.N = mul(input.N, (float3x3)World);
  output.T = float4( mul(input.T.xyz, (float3x3)World), input.T.w);
  output.Uv = input.Uv;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
void PS( VS_OUTPUT input
, out float4 o_albedo : SV_Target0
, out float4 o_normal : SV_Target1
, out float1 o_depth  : SV_Target2
, out float4 o_self_illum : SV_Target3
) {
  float4 albedo_color = txAlbedo.Sample(samLinear, input.Uv);

  o_albedo.xyz = albedo_color.xyz * ObjColor.xyz;
  o_albedo.a = txMetallic.Sample(samLinear, input.Uv).r;

  // Normal mapping
  float4 N_tangent_space = txNormal.Sample(samLinear, input.Uv);  // Between 0..1
  N_tangent_space.xyz = N_tangent_space.xyz * 2 - 1.;                           // Between -1..1

  float3x3 TBN = computeTBN( input.N, input.T );
  float3 N = mul( N_tangent_space.xyz, TBN );   // Normal from NormalMap
  N = normalize(N);

  // Save roughness in the alpha coord of the N render target
  float roughness = txRoughness.Sample(samLinear, input.Uv).r;
  o_normal = encodeNormal(N, roughness);

  float3 cam2obj = input.WorldPos.xyz - CameraPosition.xyz;
  float  linear_depth = dot( cam2obj, CameraFront ) / CameraZFar;

  o_depth = linear_depth;

  o_self_illum = txEmissive.Sample(samLinear,input.Uv);
  o_self_illum *= o_self_illum.a;
}