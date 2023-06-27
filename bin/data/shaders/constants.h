#include "platform.h"

#define CTE_BUFFER_SLOT_CAMERAS     0
#define CTE_BUFFER_SLOT_OBJECT      1
#define CTE_BUFFER_SLOT_SHARED      2
#define CTE_BUFFER_SLOT_DEBUG_LINE  3
#define CTE_BUFFER_SLOT_SKIN_BONES  4
#define CTE_BUFFER_SLOT_LIGHT       5
#define CTE_BUFFER_SLOT_BLUR        6
#define CTE_BUFFER_SLOT_FXAA        7
#define CTE_BUFFER_SLOT_FOCUS       8
#define CTE_BUFFER_SLOT_BLOOM       9
#define CTE_BUFFER_SLOT_CHR_ABR     10
#define CTE_BUFFER_SLOT_MATERIAL    11
#define CTE_BUFFER_SLOT_UI          12
#define CTE_BUFFER_SLOT_COMP_BUFFERS 13
#define CTE_BUFFER_SLOT_PARTICLES   13

// TS = TEXTURE_SLOT
#define TS_ALBEDO            0
#define TS_NORMAL            1
#define TS_METALLIC          2
#define TS_ROUGHNESS         3
#define TS_EMISSIVE          4

#define TS_PROJECTOR         5
#define TS_LIGHT_SHADOW_MAP  6
#define TS_ENVIRONMENT_MAP   7
#define TS_IRRADIANCE_MAP    8

#define TS_DEFERRED_ALBEDOS  9
#define TS_DEFERRED_NORMALS  10
#define TS_DEFERRED_LINEAR_DEPTH 11
#define TS_DEFERRED_SELF_ILLUM   12
#define TS_DEFERRED_ACC_LIGHTS   13
#define TS_DEFERRED_AO           14

#define TS_NOISE_MAP         16

#define TS_LUT_COLOR_GRADING  17

// ---------------------------------------------
// Mixing material extra texture slots
#define TS_MIX_BLEND_WEIGHTS          18

#define TS_FIRST_SLOT_MATERIAL_0      TS_ALBEDO
#define TS_FIRST_SLOT_MATERIAL_1      TS_ALBEDO1
#define TS_FIRST_SLOT_MATERIAL_2      TS_ALBEDO2

#define TS_ALBEDO1 20
#define TS_NORMAL1 21
// #define TS_METALLIC1 22
// #define TS_ROUGHNESS1 23
#define TS_ALBEDO2 24
#define TS_NORMAL2 25
// #define TS_METALLIC2 26
// #define TS_ROUGHNESS2 27



// -------------------------------------------------
// Render Outputs. Must be in sync with module_render.cpp
#define RO_COMPLETE           0
#define RO_ALBEDO             1
#define RO_NORMAL             2
#define RO_NORMAL_VIEW_SPACE  3
#define RO_ROUGHNESS          4
#define RO_METALLIC           5
#define RO_WORLD_POS          6
#define RO_LINEAR_DEPTH       7
#define RO_AO                 8

// -------------------------------------------------
#define MAX_SUPPORTED_BONES        128

#define PI 3.14159265359f

SHADER_CTE_BUFFER(TCtesCamera, CTE_BUFFER_SLOT_CAMERAS)
{
  matrix Projection;
  matrix View;
  matrix ViewProjection;
  matrix InverseViewProjection;
  matrix CameraScreenToWorld;
  matrix CameraProjWithOffset;
  float3 CameraFront;
  float  CameraZFar;
  float3 CameraPosition;
  float  CameraZNear;
  float  CameraTanHalfFov;
  float  CameraAspectRatio;
  float2 CameraInvResolution;
  float3 CameraLeft;
  float  CameraDummy1;
  float3 CameraUp;
  float  CameraDummy2;
};

SHADER_CTE_BUFFER(TCtesObject, CTE_BUFFER_SLOT_OBJECT)
{
  matrix World;
  float4 ObjColor;
  float4 CustomValues;
};

SHADER_CTE_BUFFER(TCtesShared, CTE_BUFFER_SLOT_SHARED)
{
  float  GlobalWorldTime;
  float  GlobalDeltaTime;
  float  GlobalDeltaUnscaledTime;
  float  GlobalDummy1;

  float  GlobalDummy2;
  int    GlobalRenderOutput;
  float  GlobalAmbientBoost;
  float  GlobalExposureAdjustment;

  float GlobalFXAmount;
  float GlobalFXVal1;
  float GlobalFXVal2;
  float GlobalFXVal3;

  float  GlobalLUTAmount;
  float  CoffeeRatio;
  float  Actual_dt;
  float  Globaldumm3;

  float ao_power;
  float shadow_ramp;
  float color_intensity;
  float brush_size;

  float brush_rotation;
  float specular_ramp;
  float specular_brush_ramp;
  float specular_strength;

};

SHADER_CTE_BUFFER(TCtesDebugLine, CTE_BUFFER_SLOT_DEBUG_LINE)
{
  // The float4 for the positions is to enforce alignment
  float4 DebugSrc;
  float4 DebugDst;
  float4 DebugColor;
};

SHADER_CTE_BUFFER(TCteSkinBones, CTE_BUFFER_SLOT_SKIN_BONES)
{
  matrix Bones[MAX_SUPPORTED_BONES];
};

SHADER_CTE_BUFFER(TCtesLight, CTE_BUFFER_SLOT_LIGHT)
{
  float4 LightColor;
  float3 LightPosition;
  float  LightIntensity;
  matrix LightViewProjOffset;
  float  LightShadowStep;
  float  LightShadowInverseResolution;
  float  LightShadowStepDivResolution;
  float  LightRadius;
  float3 LightFront;      // For the sun
  float  LightDummy2;
};

SHADER_CTE_BUFFER(TCtesBlur, CTE_BUFFER_SLOT_BLUR)
{
  float4 blur_w;        // weights
  float4 blur_d;        // distances for the 1st, 2nd and 3rd tap
  float2 blur_step;     // Extra modifier
  float2 blur_center; // To keep aligned x4
};

SHADER_CTE_BUFFER(TCtesFXAA, CTE_BUFFER_SLOT_FXAA)
{
  float screenWidth;
  float screenHeight;
  float sub_pix;
  float edge_threshold;
  float edge_threshold_min;
  float dumm1;
  float dumm2;
  float dumm3;
};

SHADER_CTE_BUFFER(TCtesDamage, CTE_BUFFER_SLOT_CHR_ABR)
{
  float grade;
  float Dummy1;
  float Dummy2;
  float Dummy3;
};

SHADER_CTE_BUFFER(TCtesChromaticAberration, CTE_BUFFER_SLOT_CHR_ABR)
{
  float WaterEffectSpeed;
  float DistortionSpeed;
  float DistortionAmount;
  float Dummy;
}; 

SHADER_CTE_BUFFER(TCtesFocus, CTE_BUFFER_SLOT_FOCUS)
{
  float focus_z_center_in_focus;
  float focus_z_margin_in_focus;
  float focus_transition_distance;
  float focus_modifier;
};

SHADER_CTE_BUFFER(TCtesBloom, CTE_BUFFER_SLOT_BLOOM)
{
  float4 bloom_weights;
  float  bloom_threshold_min;
  float  bloom_threshold_max;
  float  emissive_bloom_factor;
  float  bloom_pad1;
};

SHADER_CTE_BUFFER(TCtesMaterial, CTE_BUFFER_SLOT_MATERIAL) 
{
  // float  scalar_roughness;
  // float  scalar_metallic;
  // float  scalar_irradiance_vs_mipmaps;
  // float  material_dummy;

  float  mix_boost_r;
  float  mix_boost_g;
  float  mix_boost_b;
  float  material_dummy2;
};

SHADER_CTE_BUFFER(TCtesUI, CTE_BUFFER_SLOT_UI)
{
  float2 UIminUV;
  float2 UImaxUV;
  float4 UItint;
};

SHADER_CTE_BUFFER(TCtesParticles, CTE_BUFFER_SLOT_PARTICLES)
{
  float2 psystem_frame_size;
  float2 psystem_nframes;
  float4 psystem_colors_over_time[8];
  // Stored as float4 because hlsl will access it as array
  // Only the .x is currently being used in the shader particles.inc
  float4 psystem_sizes_over_time[8];

  float  emitter_time_between_spawns;         // 1.0
  float3 emitter_center;            

  uint   emitter_num_particles_per_spawn;     // 1
  float3 emitter_dir;

  float  emitter_center_radius;
  float  emitter_dir_aperture;
  float2 emitter_speed;           // min/max

  float2 emitter_duration;        // min/max
  float2 emitter_dummy;
};


