#include "common.hlsli"
#include "reflections.hlsli"
#include "metalic_roughness_ambient.hlsli"

struct PSInput
{
    float4 hpos : SV_POSITION;
    float4 texcoord : TEXCOORD0;
};

float4 main(PSInput I) : SV_Target
{
    float4 depth = s_position.GatherRed(smp_nofilter, I.texcoord.xy);
	
	float MinDepth = min(min(depth.x, depth.y), min(depth.z, depth.w));
	float MaxDepth = max(max(depth.x, depth.y), max(depth.z, depth.w));
	float AvgDepth = dot(depth, 0.25f);
	
	return float4(MinDepth, MaxDepth, AvgDepth, 1.0f);
}

