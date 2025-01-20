#include "common.hlsli"

struct v2p
{
    float2 tc0 : TEXCOORD0; // base
    float4 c : COLOR0; // diffuse
};

// Pixel
float4 main(v2p I) : COLOR
{
    return I.c * s_base.Sample(smp_base, I.tc0);
}
