
#ifdef WIN32

#define SHADER_CTE_BUFFER(xName,idx) struct xName 
#define matrix MAT44
#define float4 VEC4
#define float3 VEC3
#define float2 VEC2
#define uint   uint32_t

#else

#define SHADER_CTE_BUFFER(xName,idx) cbuffer xName : register(b ## idx)
#define TEXTURE_SLOT(idx)                          : register(t ## idx)

#endif

