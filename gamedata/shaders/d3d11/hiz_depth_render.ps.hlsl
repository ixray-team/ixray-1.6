#include "common.hlsli"
#include "reflections.hlsli"
#include "metalic_roughness_ambient.hlsli"

struct PSInput
{
    float4 hpos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput I) : SV_Target
{
    float4 depth = s_position.GatherRed(smp_nofilter, I.texcoord.xy);
	
	float MinDepth = min(min(depth.x, depth.y), min(depth.z, depth.w));
	float MaxDepth = max(max(depth.x, depth.y), max(depth.z, depth.w));
	
	return float4(MinDepth, MaxDepth, frac(MinDepth), 1.0f);
}

