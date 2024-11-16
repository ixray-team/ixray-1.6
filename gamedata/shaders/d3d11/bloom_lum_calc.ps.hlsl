#include "common.hlsli"

struct PSInput
{
    float4 hpos : SV_Position;
    float4 texcoord : TEXCOORD0;
};

uniform Texture2D b_image;
uniform Texture2D p_image;
float4 adapt_params;


float main(PSInput I) : SV_Target
{
    float res = 0.f;
    float2 center = I.texcoord.xy ;

    float LumaCurr = 0.f;         // current avg luminance
    float MaxLumaCurr = 0.f;      // current max luminance over 16x16 input
    float MinLumaCurr = 0.f;
    float tempCurr = 0.f;
    
    float Sensitivity = 0.00f;    // factor to lerp avg and max
    float2 temp;
    float weight;               // weight factor for each luma pixel in input 16x16
    float weightsumm;
    
    // here we perform weighed average summ and scan for local maximum
    // weight is basicly vignette function (its rather cheap to compute in runtime instead of making prebaked array of values
    
    for (float x = 0.0625f; x < 1.f; x += 0.0625f)
    {
        for (float y = 0.0625f; y < 1.f; y += 0.0625f)
        {
            // sample location of 16x16 tex
            tempCurr = b_image.Sample(smp_rtlinear, float2(x, y)).r;
            // find max
            MaxLumaCurr = max(tempCurr, MaxLumaCurr);
            MinLumaCurr = min(tempCurr, MinLumaCurr);
            // vignette factor calc
            //temp = float2(x, y);
            //temp *= 1.0 - temp.yx;
            //weight = temp.x * temp.y * 10.0;
            //weight = fast_sqrt(weight);
            // apply weight to our summ
            LumaCurr += tempCurr;
            //weightsumm += weight;
        }

    }
    
    LumaCurr *= 1.f / (16.f * 16.f);   // if we remove weight use this
    
    // now we take into account brighest luma sample, since eyes would to to accomodate brightness
    LumaCurr = lerp(LumaCurr, MaxLumaCurr, Sensitivity);
    
    //LumaCurr = exp(LumaCurr);
    
    // sample previous frame result
    //float speed = lerp(1, 120, PostProcessingConstantsBuffer.AdaptationTime);
    float LumaPrev = p_image.Sample(smp_rtlinear, 0.5f).r;
    //float fps = 1 - pow(0.98f, 60 * GlobalConstantsBuffer.GTimer.y);
    
    //LumaCurr = lerp(LumaPrev, LumaCurr, saturate(fps));
    
    LumaCurr = LumaPrev + (LumaCurr - LumaPrev) * (1.f - exp(-1.0 * adapt_params.x));
    
    return LumaCurr;
}