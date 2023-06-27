//--------------------------------------------------------------------------------------
#include "common.fx"
#include "particles.inc"

#define NUM_PARTICLES_PER_THREAD_GROUP  16
#define OFFSET_NUM_PARTICLES_TO_DRAW    4

// --------------------------------------------------------------
struct TInstance {
  float3 pos;
  uint   unique_id;
  
  float3 dir;
  float  dummy1;

  float3 acc;
  float  dummy2;

  float3 prev_pos;
  float  dummy3;

  float4 color;

  float  scale;
  float  time_normalized;     // 0..1 When time reaches zero the particles dies.
  float  time_factor;         // real time adds to time_normalized by this factor
  float  dummy4;
};

struct TSystem {
  float  time_to_next_spawn;
  uint   num_particles_to_update;
  uint   next_unique_id;
  uint   dummy;
};

// ----------------------------------------------------------
// The spawn fn to customize
TInstance spawnParticle( uint unique_id ) {
  
  float  rnd1 = rand(unique_id + 7654) * 2.0 - 1.0;
  float  rndz = rand(unique_id + 86231) * 2.0 - 1.0;
  float2 rnd2 = hash2( unique_id ) * 2.0 - 1.0;
  float2 rnd3 = hash2( unique_id + 57 );
  float2 rnd4 = hash2( unique_id + 75 ) * 2.0 - 1.0;
  float  rnd5 = rand(unique_id + 75) * 10.0f;
  float  rnd6 = rand(unique_id + 7654) * 0.75f;
  float  duration = emitter_duration.x + ( emitter_duration.y - emitter_duration.x ) * rnd3.x;
  float  speed  = emitter_speed.x + ( emitter_speed.y - emitter_speed.x ) * rnd3.y;

  TInstance p;
  p.pos = emitter_center;
  p.prev_pos = p.pos;
  p.acc = float3(0,0,0);
  p.dir = emitter_dir;
  p.unique_id = unique_id;
  p.time_normalized = 0.0;
  p.time_factor = 1.0 / duration;
  p.scale = 0.1;
  p.color = float4(1,1,0,1);
  p.dummy1 = rnd5;
  p.dummy2 = rnd6;
  p.dummy3 = rnd1;
  p.dummy4 = emitter_dir_aperture;

  p.dir *= speed;

  return p;
}

// ----------------------------------------------------------
// The update fn to customize
void updateParticle( inout TInstance p ) {
  p.prev_pos = p.pos;
  p.color = sampleColor( p.time_normalized );
  p.scale = sampleScale( p.time_normalized );
  p.dir += p.acc * GlobalDeltaTime;
  p.pos += p.dir * GlobalDeltaTime;
  /*if( p.pos.y < 0 ) {
    p.pos.y = -p.pos.y;
    p.dir.y = -p.dir.y;
  }*/
}


[numthreads(1, 1, 1)]
void cs_particles_onom_spawn( 
  uint thread_id : SV_DispatchThreadID,
  RWStructuredBuffer<TInstance> instances : register(u0),
  RWStructuredBuffer<TSystem> system  : register(u1),
  RWByteAddressBuffer indirect_draw   : register(u2),
  RWByteAddressBuffer indirect_update : register(u3)
) {

  // We start from the num particles left in the prev frame
  uint   nparticles_active = indirect_draw.Load( OFFSET_NUM_PARTICLES_TO_DRAW );

  // Clear num instances for indirect draw call. At offset 4, set zero
  indirect_draw.Store( OFFSET_NUM_PARTICLES_TO_DRAW, 0 );

  // Get access to the max capacity of the buffer
  uint max_elements, bytes_per_instance;
  instances.GetDimensions( max_elements, bytes_per_instance );

  // Can we spawn particles?
  if( nparticles_active + emitter_num_particles_per_spawn < max_elements ) {

    system[0].time_to_next_spawn -= GlobalDeltaTime;
    /*if( system[0].time_to_next_spawn < 0) {
      system[0].time_to_next_spawn += emitter_time_between_spawns;*/

      // Spawn N
      uint unique_id = system[0].next_unique_id;
      for( uint i=0; i<emitter_num_particles_per_spawn; ++i ) {
        instances[nparticles_active] = spawnParticle(unique_id);
        ++nparticles_active;
        ++unique_id;
      }

      system[0].next_unique_id = unique_id;
    //}
  }

  // Update DispatchIndirect 1st argument.
  uint nthread_groups = ( nparticles_active + NUM_PARTICLES_PER_THREAD_GROUP - 1 ) / NUM_PARTICLES_PER_THREAD_GROUP;
  indirect_update.Store(0, nthread_groups);
  indirect_update.Store(4, 1);
  indirect_update.Store(8, 1);
  system[0].num_particles_to_update = nparticles_active;
}

// --------------------------------------------------------------
[numthreads(NUM_PARTICLES_PER_THREAD_GROUP, 1, 1)]
void cs_particles_onom_update( 
  uint thread_id : SV_DispatchThreadID,
  StructuredBuffer<TInstance> instances : register(t0),
  StructuredBuffer<TSystem>   system    : register(t1),
  RWStructuredBuffer<TInstance> instances_active : register(u0),
  RWByteAddressBuffer indirect_draw : register(u2)
) {

  if( thread_id >= system[0].num_particles_to_update )
    return;

  TInstance p = instances[ thread_id ];

  // Has died?
  p.time_normalized += GlobalDeltaTime * p.time_factor;
  if( p.time_normalized >= 1 ) 
    return;

  // Call the specific method to update each particle
  updateParticle( p );

  // Update indirect draw call args
  uint index;
  indirect_draw.InterlockedAdd( OFFSET_NUM_PARTICLES_TO_DRAW, 1, index );

  // Save in the nexts buffer
  instances_active[index] = p;
}

//-------------------------`-------------------------------------------------------------
struct v2p {   // Vertex to pixel
  float4 Pos   : SV_POSITION;
  float2 Uv    : TEXCOORD0;
  float4 Color : COLOR;
  float  aux   : TEXCOORD1;
};

struct VS_INPUT {   // Input from billboard mesh
  float4 Pos   : POSITION;
  float2 Uv    : TEXCOORD0;
  float4 Color : COLOR;         // Not used anymore
};

//--------------------------------------------------------------------------------------
v2p VS(
  in VS_INPUT input
, in uint InstanceID : SV_InstanceID
, StructuredBuffer<TInstance> instances_active : register(t0)
)
{
  TInstance instance = instances_active[ InstanceID ];

  // orient billboard to camera
  float3 localPos = input.Pos.x * CameraLeft
                  - input.Pos.y * CameraUp;
  float3 p = instance.pos + (localPos * instance.scale) + CameraFront * 0.1f * instance.time_normalized;

  // 
  v2p output;
  output.Pos = mul( float4(p,1.0), ViewProjection );
  output.Uv = input.Uv;
  output.Color = instance.color;
  output.aux = instance.dummy4;
  return output;
}

//--------------------------------------------------------------------------------------
float4 PS(v2p input
, in uint InstanceID : SV_InstanceID
, StructuredBuffer<TInstance> instances_active : register(t0)
) : SV_Target0 {
  float2 uv = input.Uv;
  uv /= 3.0f;
  uv.x = -uv.x;

  if(input.aux >= 2.5f && input.aux <= 3.5f){
    input.aux = 3.0f;
  }else if(input.aux >= 5.5f && input.aux <= 6.5f){
    input.aux = 6.0f;
  }
  uv.y += 0.333f * (floor(input.aux / 3.0f));
  uv.x -= 0.333f * (input.aux % 3.0f);

  float4 tex = txAlbedo.Sample(samLinear,uv);
  /*tex += txAlbedo.Sample(samLinear,uv) * (input.aux >= 0.5f && input.aux < 1.5f);
  tex += txNormal.Sample(samLinear,uv) * (input.aux >= 1.5f && input.aux < 2.5f);
  tex += txMetallic.Sample(samLinear,uv) * (input.aux >= 2.5f && input.aux < 3.5f);
  tex += txRoughness.Sample(samLinear,uv) * (input.aux >= 3.5f && input.aux< 4.5f);*/


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
  return tex * input.Color;
}
