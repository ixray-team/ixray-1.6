#include "common.hlsli"

#include "metalic_roughness_light.hlsli"
#include "metalic_roughness_ambient.hlsli"

Texture2D<float> s_occ;

struct _input
{
    float4 tc0 : TEXCOORD0;
    float2 tcJ : TEXCOORD1;
    float4 pos2d : SV_POSITION;
};

float4 main(_input I) : SV_Target
{
    IXrayGbuffer O;
    GbufferUnpack(I.tc0.xy, I.pos2d.xy, O);
    float3 Light = s_accumulator.Load(int3(I.pos2d.xy, 0)).xyz;

#ifdef USE_R2_STATIC_SUN
    Light += O.SSS * DirectLight(Ldynamic_color, Ldynamic_dir.xyz, O.Normal, O.View.xyz, O.Color, O.Metalness, O.Roughness);
#endif

    float Occ = s_occ.SampleLevel(smp_rtlinear, I.tc0.xy, 0.0f).x;
    float3 Ambient = Occ * AmbientLighting(O.View, O.Normal, O.Color, O.Metalness, O.Roughness, O.Hemi);
    float3 Color = Ambient + Light;

    float Fog = saturate(O.ViewDist * fog_params.w + fog_params.x);
    Color = lerp(Color, pow(fog_color.rgb, 2.2f), saturate(Fog * Fog));

    return float4(Color, Fog * Fog);
}

