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
  float3 WorldPos : TEXCOORD1;
  float4 T        : NORMAL1;
};


float4 getNewVertPosition( float4 p, float3 N )
{
  float freqx = 1.0f + sin(GlobalWorldTime) * 1.0f;
  float freqy = 1.0f + sin(GlobalWorldTime * 1.3f) * 1.0f;
  float freqz = 1.0f + sin(GlobalWorldTime * 1.1f) * 1.0f;
  float amp = sin(GlobalWorldTime * 1.4);
  
  float f = sin(N.x * freqx + GlobalWorldTime) * sin(N.y * freqy + GlobalWorldTime) * sin(N.z * freqz + GlobalWorldTime);
  p.z += N.z * amp * f;
  p.x += N.x * amp * f;
  p.y += N.y * amp * f;
  return p;
}

// 2D Random
float random (in float2 st) {
    return frac(sin(dot(st.xy,
                         float2(12.9898,78.233)))
                 * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in float2 st) {
    float2 i = floor(st);
    float2 f = frac(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    float2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return lerp(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
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

  float4 newPos = getNewVertPosition(Pos, N);

  float4 bitangent = float4(cross(N, T.xyz),1.0f);
  float4 positionAndTangent = getNewVertPosition( Pos + T * 0.001f, N );
  float4 positionAndBitangent = getNewVertPosition( Pos + bitangent * 0.001f, N );
  float4 newTangent = ( positionAndTangent - Pos ); // leaves just 'tangent'
  newTangent = normalize(newTangent);
  float4 newBitangent = ( positionAndBitangent - Pos ); // leaves just 'bitangent'
  newBitangent = normalize(newBitangent);
  float3 newNormal = cross( newTangent.xyz, newBitangent.xyz );
  newNormal = normalize(newNormal);

  output.Pos = mul(newPos, World);
  output.WorldPos = output.Pos.xyz;
  output.Pos = mul(output.Pos, ViewProjection);
  output.N = mul(newNormal, (float3x3)World);
  output.T = float4( mul(T.xyz, (float3x3)World), T.w);
  output.Uv = Uv;
  return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
  const float freq = 0.5f; //iridiscent frequency
  const float amplitude = 10.0f; //iridiscent amplitude
  const float speed = 0.1f; //iridiscent movement bubble
  const float color_speed = 5.0f; // iridiscent color movement speed
  const int reflection = 4; //reflection term of iridiscent
  const int iridiscent_intensity = 1.2f;

  float3 N = normalize(input.N);
  float3 incident_dir = normalize(input.WorldPos - CameraPosition.xyz);
  float fresnel_term = 1 - saturate( dot( input.N, -incident_dir) );
  fresnel_term = pow(fresnel_term, reflection);

  //use the noise function
  float npatron = txAlbedo.Sample(samLinear, input.Uv * freq + float2(sin(GlobalWorldTime * speed), cos(GlobalWorldTime*speed + 0.66))).x;
  npatron = sin(amplitude*sin(npatron)) * 0.5f + 0.5f;
  npatron = pow(npatron, 100);


  float3 iridisColor = txAlbedo.Sample(samLinear, input.Uv).xyz;
  iridisColor += sin(2.*sin(iridisColor*amplitude + GlobalWorldTime*color_speed)+input.Uv.yxyy-input.Uv.yyxy*12.5) * 0.5f + 0.5f;
  iridisColor = pow(iridisColor, iridiscent_intensity);
  iridisColor = clamp(normalize(iridisColor), 0.0, 1.0);


  float2 pos = float2(input.Uv*5.0);
  float n = noise(pos);
  float3 color2 = float3(n,n,n);
  color2 += sin(2.*sin(color2*22.+GlobalWorldTime*2.)+input.Uv.yxyy-input.Uv.yyxy*.5)/12.;    // colour transform

  float3 color = lerp(ObjColor.xyz * color2, iridisColor, fresnel_term * npatron);

  return float4(color, 1.0f);
}