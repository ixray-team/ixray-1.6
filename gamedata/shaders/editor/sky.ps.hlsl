#include "common.hlsli"

struct v2p
{
    float4 factor : COLOR0;
    float3 tc0 : TEXCOORD0;
    float3 tc1 : TEXCOORD1;
};

uniform TextureCube s_sky0 : register(s0);
uniform TextureCube s_sky1 : register(s1);
uniform float3 color;

// Pixel
float4 main(v2p I) : COLOR
{
    float3 s0 = s_sky0.SampleLevel(smp_base, I.tc0, 0);
    float3 s1 = s_sky1.SampleLevel(smp_base, I.tc1, 0);
    float3 sky = I.factor * lerp(s0, s1, I.factor.w);

    return float4(sky, 1);
}
