#include "common.hlsli"
#include "mblur.hlsli"
#include "dof.hlsli"

uniform Texture2D n_bloom;



float3 CommerceToneMapping( float3 color ) 
{
    float startCompression = 0.8 - 0.04;
    float desaturation = -0.15; // - 0.15

    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return color;

    float d = 1.f - startCompression;
    float newPeak = 1.f - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1.f - 1.f / (desaturation * (peak - newPeak) + 1.f);
    return lerp(color, newPeak * 1.f, g);
}

float3 ScreenSpaceDither( float2 vScreenPos )
{
// Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR
float3 vDither = dot( float2( 171.0, 231.0 ), vScreenPos.xy + timers.w ).xxx;
vDither.rgb = frac( vDither.rgb / float3( 103.0, 71.0, 97.0 ) ) - float3( 0.5, 0.5, 0.5 );
return ( vDither.rgb / 255.0 ) * 0.375;
}

float4 main(v2p_aa_AA I) : SV_Target
{
    float3 Color = 1.f * dof(I.Tex0);
    float4 Bloom = n_bloom.Sample(smp_rtlinear, I.Tex0);
    

    IXrayGbuffer O;
    GbufferUnpack(I.Tex0, I.HPos.xy, O);

    float avg_luma = s_tonemap.Sample(smp_nofilter, float2(0.5f, 0.5f)).x;
    float loc_luma = dot(Color.rgb, LUMINANCE_VECTOR);

    avg_luma = max(avg_luma, 1e-5) * 0.98f + 0.02f; // adaptation.x, adaptation.y
    float linexp = lerp(0.178f, loc_luma, 0.00) / avg_luma; // middlegrey 0.148 or 0.178 + local contrast
    float exposure = log2(linexp) + 0.0; // preexposure 
    exposure = exp2(exposure);
    

    //Color = tonemap(Color, Scale);


/*
    luma = dot(Bloom.rgb, LUMINANCE_VECTOR);
    luma = pow(luma, 1.1); // contrast
    float3 poweredBloom = pow(Bloom.rgb, 1.3); // saturation
    powluma = dot(poweredBloom.rgb, LUMINANCE_VECTOR);
    Bloom.rgb=poweredBloom.xyz*luma/(powluma+0.0001);
*/

    float Distance = length(O.Point);
    float fog = 1.0 - exp(-Distance * 0.01f);

    float3 campos = eye_position;

    float Depth = s_position.SampleLevel(smp_nofilter, I.Tex0, 0.0).x;
    float s_view_z = depth_unpack.x / (Depth - depth_unpack.y);
    float3 Point = float3((I.Tex0 * 2.0 - 1.0) * pos_decompression_params.xy, 1.0) * s_view_z;
    //wPos.z = 
    float3 wPos = mul(m_invV, float4(Point, 1.0));
    //wPos.y = length(wPos) <= 0.0f ? 1000.0 : wPos.y;
    float3 sundir = L_sun_dir_w.xyz;
    float wDistance = length(wPos - campos);
    float sunamount = max( dot(normalize(Point), -sundir.xyz), 0.0);
    float2 heighfog_params = float2(0.0025, 0.0005);

    //float fogAmount = (heighfog_params.x / heighfog_params.y) * exp(-campos.y * heighfog_params.y) * (1.0 - exp(-wDistance*wPos.y*heighfog_params.y)) / wPos.y;

    float fogAmount = 1.0 - exp(-length(Point.xyz) * 0.002);

    fogAmount = saturate(fogAmount);
    fogAmount = pow(fogAmount, 1.0);
    //float Fog = saturate(length(O.PointReal) * fog_params.w + fog_params.x); // 1.0 - exp(-Distance * fog_params.w) + fog_params.x; //
    //Color = lerp(Color, pow(fog_color, 2.2f), saturate(Fog));

    float3 fogtint = pow(lerp( fog_color.rgb, L_sun_color, pow(sunamount,8.0) ), 1.0);

    float3 fogBloom = Bloom.rgb * fogtint.rgb;

    Color = lerp(Color, fogBloom, fogAmount);

    //Bloom.rgb -= Color;
    //Bloom = max(Bloom, 0.0f);
    Color.rgb +=  0.05f * Bloom.rgb;

    Color *= exposure;
    //Color.rgb *= 1.0 / (Color.rgb + 1.0);
    //Color.rgb = 1 - exp(-Color.rgb * 1.0);

    float luma = dot(Color.rgb, LUMINANCE_VECTOR);
    luma = pow(luma, 1.3); // contrast 
    float3 poweredColor = pow(Color.rgb, 0.9); // saturation
    float powluma = dot(poweredColor.rgb, LUMINANCE_VECTOR);
    Color.rgb=poweredColor.xyz*luma/(powluma+0.0001);

    Color.rgb = CommerceToneMapping( 1.f * Color.rgb);

    // Align the encoded range to texel centers.
    float LUT_DIMS = 48.f;
    float3 uv = Color.rgb * ((LUT_DIMS - 1.f) / LUT_DIMS) + 0.5f / LUT_DIMS;

    Color.rgb = s_tonemap_lut.Sample(smp_rtlinear, uv).rgb;

    Color.rgb = pow(Color.rgb, 0.454545);




    Color.rgb += ScreenSpaceDither(I.HPos.xy);

    return float4(Color.rgb, 1.f);//combine_bloom(Color, Bloom);
}

