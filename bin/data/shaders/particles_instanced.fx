//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "common.fx"
#include "particles.inc"

//--------------------------------------------------------------------------------------
struct ParticleRenderData
{
  float3 Pos   : TEXCOORD2;
  float  Time  : TEXCOORD3;
  float4 Color : TEXCOORD4;     // Not used anymore
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

  // Mesh billboard info
  float4 iPos : POSITION,
  float2 Uv: TEXCOORD0,
  float4 Color : COLOR,         // Not used anymore

  // Particle instance data
  ParticleRenderData particle
)
{
  VS_OUTPUT output = (VS_OUTPUT)0;

  // orient billboard to camera
  float3 localPos = iPos.x * CameraLeft
                  + iPos.y * CameraUp;

  // Use scale
  localPos *= 4;

  // Translate to particle position
  float3 worldPos = particle.Pos + localPos;

  output.Pos = mul(float4(worldPos,1), ViewProjection);

  // We really don't need a color per vertex in the billboard: output.Color = particle.Color;
  // Not used anymore
  output.Color = Color * particle.Color;

  float t = particle.Time;
  output.Color = sampleColor( t );

  // Advance animation frame..
  // If time goes between 0..1 => nframes goes between 0..#frames-1 and 0..#frames.y
  float2 nframe = float2(t * psystem_nframes.y * psystem_nframes.x, t * psystem_nframes.y );

  // Get integer parts
  float2 ixy;
  modf( nframe, ixy );

  // Now coords goes from (#x,#y)..(#x+1,#y+1)
  Uv.xy += ixy;

  // Scale to 0..1
  output.Uv = Uv * psystem_frame_size;
  
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
  float4 texture_color = txAlbedo.Sample(samLinear, input.Uv);
  return texture_color * input.Color;
}
