#include "common.hlsli"
#include "reflections.hlsli"
#include "metalic_roughness_ambient.hlsli"

struct PSInput
{
    float4 hpos : SV_POSITION;
    float4 texcoord : TEXCOORD0;
};

int sample_mip_level;

float4 main(PSInput I) : SV_Target
{
    float3 DepthA = s_position.SampleLevel(smp_nofilter, I.texcoord.xy, sample_mip_level, int2(1, 0)).xyz;
    float3 DepthB = s_position.SampleLevel(smp_nofilter, I.texcoord.xy, sample_mip_level, int2(1, 1)).xyz;
    float3 DepthC = s_position.SampleLevel(smp_nofilter, I.texcoord.xy, sample_mip_level, int2(0, 1)).xyz;
    float3 DepthD = s_position.SampleLevel(smp_nofilter, I.texcoord.xy, sample_mip_level, int2(0, 0)).xyz;
	
	float MinDepth = min(min(DepthA.x, DepthB.x), min(DepthC.x, DepthD.x));
	float MaxDepth = max(max(DepthA.y, DepthB.y), max(DepthC.y, DepthD.y));
	
	float AvgDepth = 0.25f * (DepthA.z + DepthB.z + DepthC.z + DepthD.z);
	
	return float4(MinDepth, MaxDepth, AvgDepth, 1.0f);
}

