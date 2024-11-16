#include "common.hlsli"
#include "shadow.hlsli"

#undef USE_ULTRA_SHADOWS

#define RAY_PATH 2.0h
#define JITTER_TEXTURE_SIZE 64.0f

#define JITTER_SUN_SHAFTS

#ifdef SUN_SHAFTS_QUALITY
    #if SUN_SHAFTS_QUALITY == 1
        // #define FILTER_LOW
        #define RAY_SAMPLES 20
    #elif SUN_SHAFTS_QUALITY == 2
        // #define FILTER_LOW
        #define RAY_SAMPLES 20
    #elif SUN_SHAFTS_QUALITY == 3
        // #define FILTER_LOW
        #define RAY_SAMPLES 40
    #endif
#endif

Texture2D s_fastnoise;
Texture2D<float> half_depth;

float4 volume_range; //	x - near plane, y - far plane
float4 sun_shafts_intensity;

float4 main(v2p_TL I) : SV_Target
{
#ifndef SUN_SHAFTS_QUALITY
    return float4(0, 0, 0, 0);
#else //	SUN_SHAFTS_QUALITY
    IXrayGbuffer O;
    GbufferUnpack(I.Tex0.xy, I.HPos.xy, O);

    float3 P = O.Point;
	float intensity = sun_shafts_intensity;
    float density = 0.0;

    float jitter = s_fastnoise.Load(int3(frac(I.HPos.xy / 128) * 128 , 0)).x;

	//Start and end positions, shadow projection space
	//We don't need to divide it by W, as we are working with orthographic matrix
	float4 ray_origin = mul(m_shadow, float4(0.0, 0.0, 0.0, 1.0));
	float4 ray_direction = mul(m_shadow, float4(P, 1.0));

    //Ray increment
    float3 ray_increment = (ray_direction - ray_origin) / RAY_SAMPLES;
	
    //Initial ray position. The first ray is dithered to counteract banding artifacts
	float3 ray_position = ray_origin.xyz + ray_increment.xyz * jitter;

	//Total light
	float total_light = 0.0;  

    [unroll] for(int i = 0; i < RAY_SAMPLES; i++)
    {
        density = exp(-length(ray_position) * 0.66f);
		//Apply constant bias to prevent light leaks
		total_light += density * s_smap.SampleCmpLevelZero(smp_smap, ray_position.xy, ray_position.z + 0.000098).x;
		ray_position += ray_increment;
    }
    
    total_light /= RAY_SAMPLES;

    float2 fog = 1.0 - exp(-length(P.xyz) * float2(0.005, 0.005)); // fog param.xy
    float fSturation = dot(normalize(P), -Ldynamic_dir.xyz);
    float Ro = 0.07957747f * (0.5f + 4.5f * pow((1.0 + fSturation) * 0.5f, 8.0));

    float4 localLcol = pow(Ldynamic_color, 2.2);
    float4 localFcol = pow(fog_color, 2.2);

    float4 superfogcolor = lerp(localFcol, localLcol, (0.1 + 0.9 * total_light) * Ro);

    float4 final_color = lerp(0.f, superfogcolor, fog.x ) + total_light * localLcol * fog.y * Ro;
    final_color.a = 1.0;
	//Color and output
	//return float4(total_light * Ldynamic_color.xyz, 1.0);

    return final_color;

    //return float4(jitter, jitter, jitter, 1.0);


    /*
    res /= RAY_SAMPLES;

    float fSturation = dot(normalize(P), -Ldynamic_dir.xyz);
    float fSturation2 = 0.4f * fSturation + 0.6f;
    float Ro = 0.07957747f * (0.5f + 4.5f * pow((1.0 + fSturation) * 0.5f, 8.0));
    //float2 noise = J0.yz;
    float2 fog = 1.0 - exp(-length(P.xyz) * float2(0.001, 1.0)); // fog param.xy
    //float fog2 = exp(-length(P.xyz) * 0.01);
    fog = saturate(fog);
    float4 superfogcolor = lerp(fog_color, Ldynamic_color, (0.1 + 0.9 * res) * Ro);

    float4 final_color = lerp(0.f, superfogcolor, fog.x ) + res * Ldynamic_color * fog.y * Ro;
    */
//============== orig ======================================================================
    //float fSturation = dot(normalize(P), -Ldynamic_dir.xyz);
    //	Normalize dot product to
    //fSturation = 0.4f * fSturation + 0.6f;
    //fog = saturate(length(P.xyz) * fog_params.w + fog_params.x);
    //res = lerp(res, max_density, fog);
    //res *= fSturation;

    //return res * Ldynamic_color;
 //==========================================================================================   


    //return final_color;
#endif // SUN_SHAFTS_QUALITY
}

