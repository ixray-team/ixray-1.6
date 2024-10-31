#include "common.hlsli"

struct PSInput
{
    float4 hpos : SV_Position;
    float4 texcoord : TEXCOORD0;
};

uniform Texture2D b_image;
float4 adapt_params;

static const float2 offsets2[16] =
{
    float2(-0.375, -0.375), float2(-0.125, -0.375), float2(0.125, -0.375), float2(0.375, -0.375),
    float2(-0.375, -0.125), float2(-0.125, -0.125), float2(0.125, -0.125), float2(0.375, -0.125),
    float2(-0.375, 0.125),  float2(-0.125, 0.125),  float2(0.125, 0.125),  float2(0.375, 0.125),
    float2(-0.375, 0.375),  float2(-0.125, 0.375),  float2(0.125, 0.375),  float2(0.375, 0.375)
};

float main(PSInput I) : SV_Target
{
    float res = 0.f;
    float2 center = I.texcoord.xy ;
    float x = adapt_params.z;
    float y = adapt_params.w;

    for (int i = 0; i < 16; i++)
    {
        res += b_image.Sample(smp_rtlinear, center + offsets2[i] * float2(x,y)).r;
    }
    res *= 0.0625;

    return res;
}