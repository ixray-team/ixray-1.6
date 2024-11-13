#include "common.hlsli"
#include "reflections.hlsli"

struct PSInput
{
    float4 hpos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float IntersectAABB(float4 Dir, float4 Org, float4 Box) {
	float3 RcpDir = rcp(Dir);
	
	float3 TNeg = (Box - Org) * RcpDir;
	float3 TPos = -RcpDir * (Box + Org);
	
	return max(min(TNeg.x, TPos.x), max(min(TNeg.y, TPos.y), min(TNeg.z, TPos.z)));
}

float HistoryClamp(float4 History, float4 Filtered, float4 aabb_min, float4 aabb_max) {
	float4 Min = min(Filtered, min(aabb_min, aabb_max));
	float4 Max = max(Filtered, max(aabb_min, aabb_max));
	
	float4 Avg2 = Max + Min;
	float4 Dir = Filtered - History;
	
	float4 Org = History - Avg2 * 0.5f;
	float4 Scale = Max - Avg2 * 0.5f;
	
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

void history_clamp(inout float4 history, float4 aabb_min, float4 aabb_max, float4 low_pass_center) {
	float ClampBlend = HistoryClamp(history, low_pass_center, aabb_min, aabb_max);
	history = lerp (history, low_pass_center,  ClampBlend);
}

uniform float4 scaled_screen_res;

float4 main(PSInput I) : SV_Target
{
    IXrayGbuffer O;
    GbufferUnpack(I.texcoord.xy, I.hpos.xy, O);
	
	I.texcoord.xy = I.hpos.xy * scaled_screen_res.zw;
	
	if(O.Depth > 0.9999f) {
		return 0.0f;
	}
	
	// O.Roughness = 0.0f;
	
	float4 SSLR0 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(+1, +0));
	float4 SSLR1 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-0, +1));
	float4 SSLR2 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-1, -0));
	float4 SSLR3 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0, int2(-0, -1));
	
	float4 SSLR4 = s_image.SampleLevel(smp_nofilter, I.texcoord, 0);
	
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
	SSLRMain.w = O.Depth;
	
	float3 Point = gbuf_unpack_position(I.texcoord.xy, O.PointReal.z);
	float3 View = normalize(Point);
	
	float3 ReflectPoint = View * SSLRBoxMaxPos.w + Point;
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
	
	float W = 1.0f - saturate(length(I.texcoord.xy - PrevDiffuseUV.xy) * 10.0f);
	W *= 1.0f - saturate(10.0f * abs(SSLR_OldDiffyse.w - O.Depth) / max(SSLR_OldDiffyse.w, O.Depth));
	W *= GetBorderAtten(PrevDiffuseUV, 0.001f);
	
	float4 SSLR_Diffuse = lerp(SSLRMain, SSLR_OldDiffyse, W * 0.98f);
    float4 SSLR_OldSpecular = s_refl.SampleLevel(smp_rtlinear, PrevSpecularUV.xy, 0.0f);
	
	W = 1.0f - saturate(length(I.texcoord.xy - PrevSpecularUV.xy) * 10.0f);
	W *= 1.0f - saturate(abs(SSLR_OldSpecular.w - O.Depth) * 10);
	W *= GetBorderAtten(PrevSpecularUV, 0.001f);
	
	history_clamp(SSLR_OldSpecular, SSLRBoxMin, SSLRBoxMax, SSLR_Diffuse);
	float4 SSLR_Specular = lerp(SSLRMain, SSLR_OldSpecular, W * 0.9f);
	float Fade = smoothstep(0.0f, 0.01f, SSLRBoxMax.w);
	
	SSLRMain = lerp(SSLR_Specular, SSLR_Diffuse, O.Roughness * Fade);
	SSLRMain.w = O.Depth;
	
	return SSLRMain;
}

