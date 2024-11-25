#include "common.hlsli"
#include "reflections.hlsli"

struct PSInput
{
    float4 hpos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float IntersectAABB(float3 Dir, float3 Org, float3 Box) {
	float3 RcpDir = rcp(Dir);
	
	float3 TNeg = (Box - Org) * RcpDir;
	float3 TPos = -RcpDir * (Box + Org);
	
	return max(min(TNeg.x, TPos.x), max(min(TNeg.y, TPos.y), min(TNeg.z, TPos.z)));
}

float HistoryClamp(float3 History, float3 Filtered, float3 aabb_min, float3 aabb_max) {
	float3 Min = min(Filtered, min(aabb_min, aabb_max));
	float3 Max = max(Filtered, max(aabb_min, aabb_max));
	
	float3 Avg2 = Max + Min;
	float3 Dir = Filtered - History;
	
	float3 Org = History - Avg2 * 0.5f;
	float3 Scale = Max - Avg2 * 0.5f;
	
	return saturate(IntersectAABB(Dir, Org, Scale));
}

void sort(inout float4 a1, inout float4 a2) {
	float4 t = min(a1, a2);
	a2 = max(a1, a2);
	a1 = t;
}

float4 median3(float4 a1, float4 a2, float4 a3) {
	sort(a2, a3);
	sort(a1, a2);
	
	return min(a2, a3);
}

float4 median5(float4 a1, float4 a2, float4 a3, float4 a4, float4 a5) {
	sort(a1, a2);
	sort(a3, a4);
	sort(a1, a3);
	sort(a2, a4);
	
	return median3(a2, a3, a5);
}

float4 median9(float4 a1, float4 a2, float4 a3, float4 a4, float4 a5, float4 a6, float4 a7, float4 a8, float4 a9) {
	sort(a1, a2);
	sort(a3, a4);
	sort(a5, a6);
	sort(a7, a8);
	sort(a1, a3);
	sort(a5, a7);
	sort(a1, a5);
	
	sort(a3, a5);
	sort(a3, a7);
	sort(a2, a4);
	sort(a6, a8);
	sort(a4, a8);
	sort(a4, a6);
	sort(a2, a6);
	
	return median5(a2, a4, a5, a7, a9);
}

uniform float4 scaled_screen_res;

float4 main(PSInput I) : SV_Target
{
    IXrayGbuffer O;
    GbufferUnpack(I.texcoord.xy, I.hpos.xy, O);
	
	float4 SSLR4 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0);
	
	if(O.Depth >= 1.0f) {
		return float4(SSLR4.xyz, O.Depth);
	}
	
	O.Roughness = 1.0f;
	
	float4 SSLR0 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(+1, +0));
	float4 SSLR1 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-0, +1));
	float4 SSLR2 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-1, -0));
	float4 SSLR3 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-0, -1));
	
	float4 SSLR5 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(+1, +1));
	float4 SSLR6 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-1, +1));
	float4 SSLR7 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-1, -1));
	float4 SSLR8 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-1, -1));
	
	float4 SSLRBoxMinPos = min(SSLR0, min(SSLR2, min(SSLR6, SSLR8)));
	float4 SSLRBoxMaxPos = max(SSLR0, max(SSLR2, max(SSLR6, SSLR8)));
	
	float4 SSLRBoxMin = min(SSLR1, min(SSLR3, min(SSLR5, SSLR7)));
	float4 SSLRBoxMax = max(SSLR1, max(SSLR3, max(SSLR5, SSLR7)));
	
	SSLRBoxMinPos = min(SSLRBoxMin, SSLRBoxMinPos);
	SSLRBoxMaxPos = max(SSLRBoxMax, SSLRBoxMaxPos);
	
	SSLRBoxMin = SSLRBoxMin * 0.5f + SSLRBoxMinPos * 0.5f;
	SSLRBoxMax = SSLRBoxMax * 0.5f + SSLRBoxMaxPos * 0.5f;
	
	float4 SSLRMain = median9(SSLR0, SSLR1, SSLR2, SSLR3, SSLR4, SSLR5, SSLR6, SSLR7, SSLR8); 
	
	float3 Point = gbuf_unpack_position(I.texcoord.xy, O.PointReal.z);
	float3 View = normalize(Point);
	
	float3 ReflectPoint = View.xyz * SSLRMain.w + Point.xyz;
	float2 PrevDiffuseUV = I.texcoord.xy + s_velocity.SampleLevel(smp_rtlinear, I.texcoord.xy, 0).xy * float2(-0.5f, 0.5f);
	
	float4 PrevSpecularUV = mul(m_VP_old, float4(mul(m_invV, float4(ReflectPoint, 1.0f)).xyz, 1.0f));
	PrevSpecularUV.xy = PrevSpecularUV.xy / PrevSpecularUV.w * float2(0.5f, -0.5f) + 0.5f;
	// PrevSpecularUV.xy = I.texcoord.xy;
	
	// float4 PrevUV = mul(m_VP_old, float4(mul(m_invV, float4(Point, 1.0f)).xyz, 1.0f));
	// PrevUV.xy = PrevUV.xy / PrevUV.w * float2(0.5f, -0.5f) + 0.5f;
	// // PrevDiffuseUV = PrevUV.xy;
	// PrevSpecularUV.xy = PrevDiffuseUV - PrevUV.xy + PrevSpecularUV.xy;
	
	PrevSpecularUV.xy = O.Depth > 0.02f ? PrevSpecularUV.xy : PrevDiffuseUV.xy;
	
    float4 SSLR_OldDiffyse = s_refl.SampleLevel(smp_rtlinear, PrevDiffuseUV.xy, 0.0f);
	
	float Fade = SSLRBoxMin.w < 1000.0f ? 1.0f : 0.8f;
	
	float W = saturate(1.1f - saturate(length(I.texcoord.xy - PrevDiffuseUV.xy) * 10.0f));
	W *= 1.0f - saturate(10.0f * abs(SSLR_OldDiffyse.w - O.Depth));
	W *= GetBorderAtten(PrevDiffuseUV, 0.001f) * Fade;
	
	SSLR_OldDiffyse = lerp(SSLR4, SSLR_OldDiffyse, 0.98f);
	
	float4 SSLR_Diffuse = lerp(SSLRMain, SSLR_OldDiffyse, W);
    float4 SSLR_OldSpecular = s_refl.SampleLevel(smp_rtlinear, PrevSpecularUV.xy, 0.0f);
	
	W = saturate(1.1f - saturate(length(I.texcoord.xy - PrevSpecularUV.xy) * 10.0f));
	W *= 1.0f - saturate(abs(SSLR_OldSpecular.w - O.Depth) * 10);
	W *= GetBorderAtten(PrevSpecularUV, 0.001f) * Fade;
	
	SSLR_OldSpecular = lerp(SSLR4, SSLR_OldSpecular, 0.98f);
	
	float SpecularFactor = 1.0f - HistoryClamp(SSLR_OldSpecular.xyz, SSLR4.xyz, SSLRBoxMin.xyz, SSLRBoxMax.xyz);	
	float4 SSLR_Specular = lerp(SSLR_Diffuse, SSLR_OldSpecular, W * SpecularFactor);
	
	SSLRMain.xyz = lerp(SSLR_Specular.xyz, SSLR_Diffuse.xyz, O.Roughness);
	SSLRMain.w = O.Depth;
	
	return SSLRMain;
}

