#include "stdafx.h"

#include "r4_rendertarget.h"

D3D_VIEWPORT VP_BU = {
	0.0f,
	0.0f,
	1024.f,
	1024.f,
	0.0f,
	1.0f
};

void CRenderTarget::phase_bloom_upsample()
{
	u32 Offset = 0;
	float rW, rH, W, H;
	constexpr u32 vertex_color = color_rgba(0, 0, 0, 255);

	W = float(Device.TargetWidth / 32.0);
	H = float(Device.TargetHeight / 32.0);
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_BU.Width = W;
	VP_BU.Height = H;
	RContext->RSSetViewports(1, &VP_BU);

	u_setrt(rt_Bloom_E2, nullptr, nullptr, nullptr);
	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	RCache.set_Element(s_bloom_upsample->E[0]);
	RCache.set_c("upsample_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	//==========================================================

	W = float(Device.TargetWidth / 16.0);
	H = float(Device.TargetHeight / 16.0);
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_BU.Width = W;
	VP_BU.Height = H;
	RContext->RSSetViewports(1, &VP_BU);

	u_setrt(rt_Bloom_D2, nullptr, nullptr, nullptr);
	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	RCache.set_Element(s_bloom_upsample->E[1]);
	RCache.set_c("upsample_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	//==========================================================

	W = float(Device.TargetWidth / 8.0);
	H = float(Device.TargetHeight / 8.0);
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_BU.Width = W;
	VP_BU.Height = H;
	RContext->RSSetViewports(1, &VP_BU);

	u_setrt(rt_Bloom_C2, nullptr, nullptr, nullptr);
	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	RCache.set_Element(s_bloom_upsample->E[2]);
	RCache.set_c("upsample_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	//==========================================================

	W = float(Device.TargetWidth / 4.0);
	H = float(Device.TargetHeight / 4.0);
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_BU.Width = W;
	VP_BU.Height = H;
	RContext->RSSetViewports(1, &VP_BU);

	u_setrt(rt_Bloom_B2, nullptr, nullptr, nullptr);
	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	RCache.set_Element(s_bloom_upsample->E[3]);
	RCache.set_c("upsample_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	//==========================================================

	W = float(Device.TargetWidth / 2.0);
	H = float(Device.TargetHeight / 2.0);
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_BU.Width = W;
	VP_BU.Height = H;
	RContext->RSSetViewports(1, &VP_BU);

	u_setrt(rt_Bloom_A2, nullptr, nullptr, nullptr);
	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	RCache.set_Element(s_bloom_upsample->E[4]);
	RCache.set_c("upsample_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);
}