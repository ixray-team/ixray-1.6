#include "common.hlsli"
#include "lmodel.hlsli"

//////////////////////////////////////////////////////////////////////////////////////////
// This is the basic primitive used by convex, volumetric lights
// for example spot-lights, one face of the omni lights, etc.
//////////////////////////////////////////////////////////////////////////////////////////
// following options are available to configure compilation:
// USE_LMAP
// USE_LMAPXFORM
// USE_SHADOW
//////////////////////////////////////////////////////////////////////////////////////////

uniform float4x4 m_shadow;

uniform Texture2D s_position;
uniform Texture2D s_normal;

uniform float4 Ldynamic_color;
uniform float4 Ldynamic_pos;
uniform float4 Ldynamic_dir;

float4 main( float4 hpos : POSITION, float4 tc : TEXCOORD0) : COLOR
{
	float2 tcproj = tc.xy / hpos.w;
    float4 _P = s_position.Sample(smp_base, tcproj);

	if(_P.z < 0.001f)
		return 0.0f;

    float4 _C = s_base.Sample(smp_base, tcproj);
    float4 _N = s_normal.Sample(smp_base, tcproj);
	
	_N.xyz = normalize(_N.xyz);
	
    // ----- light-model
    float4 light = plight_local(_P.w, _P, _N, Ldynamic_pos, Ldynamic_pos.w);

    // ----- shadow
    float4 P4 = float4(_P.xyz, 1.0f);
    float4 PS = mul(m_shadow, P4);
	
#ifdef USE_LMAP
    //light *= tex2Dlod(s_lmap, float4(PS.xy / PS.w, 0.0f, 0.0f));
	light *= s_lmap.Load( int3(0,0,0) ).x;
#endif

	float4 color = Ldynamic_color * light;
	color.xyz = color.xyz * _C.xyz + color.w * _C.w;

    return color;
}

