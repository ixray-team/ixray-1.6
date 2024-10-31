#include "common.hlsli"

struct PSInput
{
    float4 hpos : SV_Position;
    float4 texcoord : TEXCOORD0;
};

uniform Texture2D b_image;
uniform Texture2D t_image;
float4 upsample_params;

float4 main(PSInput I) : SV_Target
{
    float4 res = 0.f;
    float2 center = I.texcoord.xy ;
    float x = 2.0 * upsample_params.z;
    float y = 2.0 * upsample_params.w;

    float3 a = b_image.Sample (smp_rtlinear, float2 (center.x - x, center.y + y)).rgb;
    float3 b = b_image.Sample (smp_rtlinear, float2 (center.x,     center.y + y)).rgb;
    float3 c = b_image.Sample (smp_rtlinear, float2 (center.x + x, center.y +  y)).rgb;

    float3 d = b_image.Sample (smp_rtlinear, float2 (center.x - x, center.y)).rgb;
    float3 e = b_image.Sample (smp_rtlinear, float2 (center.x,     center.y)).rgb;
    float3 f = b_image.Sample (smp_rtlinear, float2 (center.x + x, center.y)).rgb;

    float3 g = b_image.Sample (smp_rtlinear, float2 (center.x - x, center.y - y)).rgb;
    float3 h = b_image.Sample (smp_rtlinear, float2 (center.x,     center.y - y)).rgb;
    float3 i = b_image.Sample (smp_rtlinear, float2 (center.x + x, center.y - y)).rgb;

    float3 upsample = 0.f;
    upsample += e * 4.f;
    upsample += (b + d + f + h) * 2.f;
    upsample += (a + c + g + i);
    upsample *= 1.f / 16.f;
    
    //downsample = prefilter(downsample, float2(0.0f, 1.0f));

    float3 prev = 0.5 * t_image.Sample(smp_rtlinear, center);

    return res = float4 (upsample + prev, 1.f);
}