//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"

Texture2D shaderTexture TEXTURE_SLOT(TS_ALBEDO);

Texture2D shaderTexture2 TEXTURE_SLOT(TS_NORMAL);

SamplerState SampleType : register(s0);

SamplerState SampleWind : register(s1);

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
  float4 Pos      : SV_POSITION;
  float2 text       : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(
  float4 Pos : POSITION,
  float2 text       : TEXCOORD0
)
{
  VS_OUTPUT output = (VS_OUTPUT)0;
  Pos.w = 1.0f;
  output.Pos = mul(Pos,World);
  output.Pos = mul(output.Pos,View);
  output.Pos = mul(output.Pos,Projection);
  
  output.text = text;
  
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
void PS_wind(VS_OUTPUT input, out float4 o_albedo : SV_Target)
{
	//float4 color = float4(1,0,0,1);
	//color = shaderTexture.Sample(SampleType,input.text);
	//color.a = 0.9;
	//return color;
	
	
	
	float4 albedo_color;
	albedo_color = shaderTexture.Sample(SampleType,input.text);
	
	float4 ice_wind;
	ice_wind = shaderTexture.Sample(SampleWind,input.text);
	
	
	float4 noise0 = txNoise.Sample(samLinear,input.text * 0.1 + 0.005 * -GlobalWorldTime * float2(0,1) * 30. );
	float a = sin(noise0.x);
	float b = cos(noise0.y);
	albedo_color = txAlbedo.Sample(samLinear, input.text * 0.9 + float2(a,b));
	

	
	
	o_albedo.xyz = albedo_color;
}


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
//#include "common.fx"

//--------------------------------------------------------------------------------------
//struct VS_OUTPUT
//{
//  float4 Pos      : SV_POSITION;
//  float3 N        : NORMAL;
// float2 Uv       : TEXCOORD0;
//  float3 WorldPos : TEXCOORD1;
//  float4 T        : NORMAL1;
//};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
//VS_OUTPUT VS(
//  float4 Pos : POSITION,
//  float3 N : NORMAL,
// float2 Uv: TEXCOORD0,
//  float4 T : NORMAL1
//)
//{
//  VS_OUTPUT output = (VS_OUTPUT)0;
//  float4 auxPos = Pos;
//  float4 noise0 = txNoise.SampleLevel(samLinear,Uv * 0.4 + 0.2 * GlobalWorldTime * float2(0,1),0);

//  auxPos = auxPos + float4(N.x, N.y, N.z , 0) * noise0;
//  auxPos.w = Pos.w;
//  output.Pos = mul(auxPos, World);
//  output.WorldPos = output.Pos.xyz;
//  output.Pos = mul(output.Pos, ViewProjection);
//  output.N = mul(N, (float3x3)World);
//  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
//  output.Uv = Uv;
//  return output;
//}



//float4 PS_wind(VS_OUTPUT input) : SV_Target
//{

//  float4 noise0 = txNoise.Sample(samLinear,input.Uv * 2 + GlobalWorldTime * float2(0,-1)) * 15. - 7.5;

  // Compute coords in screen space to sample the color under me
//  float3 wPos = input.WorldPos.xyz; //+ noise0.xyz * input.N
//  float4 viewSpace = mul( float4(wPos,1.0), ViewProjection );
//  float3 homoSpace = viewSpace.xyz / viewSpace.w;
//  float2 uv = float2( ( homoSpace.x + 1.0 ) * 0.5, ( 1.0 - homoSpace.y ) * 0.5 );

//  float4 albedo_color = txAlbedo.Sample(samClampLinear, uv);

//  float4 white = float4(1,1,1,1);
//  float4 blue = float4(0,0,1,1);
//  float4 ice_color = (blue + saturate(noise0.y)*(white - blue));
//  float amount_of_fire_color = saturate(noise0.x) * 0.9;
//  float4 base_color = amount_of_fire_color * ice_color + ( 1 - amount_of_fire_color) * albedo_color;
  
//  return base_color;
//}



//void PS_wind(VS_OUTPUT input, out float4 o_albedo : SV_Target0)
//{	
//	float4 albedo_color;
//	albedo_color = shaderTexture.Sample(SampleType,input.text);;
//	float4 noise0 = txNoise.Sample(samLinear,input.text * 0.1 + 0.05 * -GlobalWorldTime * float2(0,1) ) * 30. + 5;
//	float a = sin(noise0.x);
//	float b = cos(noise0.y);
//	albedo_color = txAlbedo.Sample(samLinear, input.text * 0.5 + float2(a,b));
//	o_albedo.xyz = albedo_color;
//}


	
  


