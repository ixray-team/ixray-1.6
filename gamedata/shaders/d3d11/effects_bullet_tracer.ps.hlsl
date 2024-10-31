#include "common.hlsli"

// Pixel
float4 main(p_TL I) : SV_Target
{
    return s_base.Sample(smp_rtlinear, I.Tex0) * I.Color * 1.0;
}
