#include "common.hlsli"
#include "reflections.hlsli"
#include "metalic_roughness_ambient.hlsli"

struct PSInput
{
    float4 hpos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PSInput I) : SV_Target
{
    IXrayGbuffer O;
    GbufferUnpack(I.texcoord.xy, I.hpos.xy, O);
	
	if(O.Depth >= 1.0f) {
		float4 Enviroment = CompureSpecularIrradance(O.View, 0.5f, 0.6f).xyzz;
		Enviroment.w = 0.0f;
		
		return Enviroment;
	}
	
	O.Roughness = 0.0f;
	
	float3 ReflectPoint = gbuf_unpack_position(I.texcoord.xy, O.PointReal.z);
	
	// O.Normal = normalize(cross(ddx(O.PointReal), ddy(O.PointReal)));
	
	float4 Enviroment = CompureSpecularIrradance(reflect(O.View, O.Normal), O.Hemi, O.Roughness).xyzz;
	Enviroment.w = 1000.0f;
	
	// if(O.Roughness > 0.5f) {
		// return Enviroment;
	// }
	
	O.Roughness = 0; //O.Roughness * 0.5f;

	float L = 0.025f;
	float3 ViewVec = O.View;
	
	float3 Jitter = normalize(Hash33(frac(ReflectPoint * timers.x) * 1234.56) - 0.5f);
	Jitter *= dot(Jitter, O.Normal) > 0.0f ? 1.0f : -1.0f;
	
	float3 RefNormal = normalize(O.Normal + Jitter * O.Roughness);
	
	float3 RefRef = reflect(ViewVec, RefNormal);
	RefRef *= dot(RefRef, O.Normal) > 0.0f ? 1.0f : -1.0f;
	
	float4 SSLR = ScreenSpaceLocalReflections(ReflectPoint, RefRef, L);
	// SSLR.w *= smoothstep(0.5f, 0.45f, O.Roughness);
	
	Enviroment.xyz = lerp(Enviroment.xyz, SSLR.xyz, SSLR.w);
	Enviroment.w = lerp(Enviroment.w, L, SSLR.w);
	
	return Enviroment;
}

