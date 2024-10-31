#include "common.hlsli"

struct PSInput
{
    float4 hpos : SV_Position;
    float4 texcoord : TEXCOORD0;
};

float3 prefilter (float3 color, float2 threshold)
{
    float thres1 = min (threshold.x, threshold.y);
    float thres2 = max (threshold.x, threshold.y);
    float brightness = (color.r + color.g + color.b) / 3.0;
    float factor = smoothstep (thres1, thres2, brightness);
    return color * factor;
}

uniform Texture2D b_image;
float4 downsample_params;

float4 main(PSInput I) : SV_Target
{
    float4 res = 0.f;
    float2 center = I.texcoord.xy ;
    float x = 0.25f * downsample_params.z;
    float y = 0.25f * downsample_params.w;

    float3 a = b_image.Sample (smp_rtlinear, float2 (center.x - 2.f * x, center.y + 2.f * y)).rgb;
    float3 b = b_image.Sample (smp_rtlinear, float2 (center.x,           center.y + 2.f * y)).rgb;
    float3 c = b_image.Sample (smp_rtlinear, float2 (center.x + 2.f * x, center.y + 2.f * y)).rgb;

    float3 d = b_image.Sample (smp_rtlinear, float2 (center.x - 2.f * x, center.y)).rgb;
    float3 e = b_image.Sample (smp_rtlinear, float2 (center.x,           center.y)).rgb;
    float3 f = b_image.Sample (smp_rtlinear, float2 (center.x + 2.f * x, center.y)).rgb;

    float3 g = b_image.Sample (smp_rtlinear, float2 (center.x - 2.f * x, center.y - 2.f * y)).rgb;
    float3 h = b_image.Sample (smp_rtlinear, float2 (center.x,           center.y - 2.f * y)).rgb;
    float3 i = b_image.Sample (smp_rtlinear, float2 (center.x + 2.f * x, center.y - 2.f * y)).rgb;

    float3 j = b_image.Sample (smp_rtlinear, float2 (center.x - x, center.y + y)).rgb;
    float3 k = b_image.Sample (smp_rtlinear, float2 (center.x + x, center.y + y)).rgb;
    float3 l = b_image.Sample (smp_rtlinear, float2 (center.x - x, center.y - y)).rgb;
    float3 m = b_image.Sample (smp_rtlinear, float2 (center.x + x, center.y - y)).rgb;

    float3 downsample = 0.f;
    downsample += e * 0.125f;
    downsample += (a + c + g + i) * 0.03125f;
    downsample += (b + d + f + h) * 0.0625f;
    downsample += (j + k + l + m) * 0.125f;
    
    //downsample = prefilter(downsample, float2(0.0f, 1.0f));

    return res = float4 (downsample, 1.f);
}