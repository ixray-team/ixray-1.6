#include "common.hlsli"

struct PSInput
{
    float4 hpos : SV_Position;
    float4 texcoord : TEXCOORD0;
};

uniform Texture2D b_image;
float4 adapt_params;

float main(PSInput I) : SV_Target
{
    float res = 0.f;
    float2 center = I.texcoord.xy ;
    float3 a = b_image.Sample (smp_rtlinear, float2 (center.x, center.y)).rgb;
    res = dot(a, LUMINANCE_VECTOR);
    res = max(res, 1e-5);
    //res = log2(res);

    return res;
}