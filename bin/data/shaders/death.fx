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
  float4 WorldPos : TEXCOORD1;
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
  float3 displa = ((txNormal.SampleLevel(samLinear,Uv*0.25f + GlobalWorldTime * 0.5f,0).xyz * 2.0f) - 1.0f) * 0.07f;
  float4 auxPos = Pos;
  auxPos.xyz += displa * N;
  output.WorldPos = mul(auxPos, World);
  output.Pos = mul(output.WorldPos, ViewProjection);
  output.N = mul(N, (float3x3)World);
  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
  output.Uv = Uv;
  return output;
}

void PS(VS_OUTPUT input
  , out float4 o_deferred : SV_Target0
  , out float4 o_shine : SV_Target1)
{

  float displ = txNormal.Sample(samLinear,input.Uv+GlobalWorldTime * 0.5f * sign(sin(GlobalWorldTime))).x * 0.1f;
  float displ2 = txNormal.Sample(samLinear,input.Uv-GlobalWorldTime * 0.5f * sign(sin(GlobalWorldTime))* 0.5f).x * 0.3;
  float3 border_color = txMetallic.Sample(samLinear,input.Uv + GlobalWorldTime);
  border_color = pow(border_color,2);


  float3 view_dir = normalize(input.WorldPos - CameraPosition);
  float  NdV = (1 - saturate(dot(input.N, -view_dir)));
  NdV = pow(NdV,2.5f);

  NdV = NdV > 0.5f + displ - displ2 ? 1.0f : 0.0f;

  float2 uv = input.Uv - float2(0.745f,0.5f);
  float a = atan2(uv.y, uv.x) / (2.0f * PI);
  float b = length(uv);
  float2 polar = float2(a,b * 0.25f)*1.5f;
  float2 polar2 = float2(a,b*1.25f)*1.5f;

  float panning = (sin(GlobalWorldTime)*0.5f + 2.5f);
  float4 center_color = txAlbedo.Sample(samLinear,(polar2) + float2(GlobalWorldTime * 0.5,GlobalWorldTime * 0.85));


  float3 final_color = border_color * ObjColor.xyz * NdV + center_color * (1 - NdV);

  o_deferred = float4(final_color,1.0f);
  o_shine = float4(final_color,1.0f);
}