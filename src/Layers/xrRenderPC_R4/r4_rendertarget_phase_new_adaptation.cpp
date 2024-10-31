#include "stdafx.h"

#include "r4_rendertarget.h"

D3D_VIEWPORT VP_NL = {
	0.0f,
	0.0f,
	1024.f,
	1024.f,
	0.0f,
	1.0f
};

void CRenderTarget::phase_new_luminance()
{
	u32 Offset = 0;
	float rW, rH, W, H;
	constexpr u32 vertex_color = color_rgba(0, 0, 0, 255);

	W = 1024.f;//float(Device.TargetWidth / 32.0);
	H = 1024.f;//float(Device.TargetHeight / 32.0);
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_NL.Width = W;
	VP_NL.Height = H;
	RContext->RSSetViewports(1, &VP_NL);

	u_setrt(rt_LUM_A, nullptr, nullptr, nullptr);
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

	RCache.set_Element(s_lum_copy->E[0]);
	RCache.set_c("adapt_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	//=================================================================

	W /= 8.f;//128
	H /= 8.f;//128
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_NL.Width = W;
	VP_NL.Height = H;
	RContext->RSSetViewports(1, &VP_NL);

	u_setrt(rt_LUM_B, nullptr, nullptr, nullptr);
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

	RCache.set_Element(s_lum_copy->E[1]);
	RCache.set_c("adapt_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	//=================================================================

	W /= 8.f;//16
	H /= 8.f;//16
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_NL.Width = W;
	VP_NL.Height = H;
	RContext->RSSetViewports(1, &VP_NL);

	u_setrt(rt_LUM_C, nullptr, nullptr, nullptr);
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

	RCache.set_Element(s_lum_copy->E[2]);
	RCache.set_c("adapt_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	//=================================================================

	W = 1.f;//1
	H = 1.f;//1
	rW = 1.0f / W;
	rH = 1.0f / H;

	VP_NL.Width = W;
	VP_NL.Height = H;
	RContext->RSSetViewports(1, &VP_NL);

	u_setrt(rt_LUM_D, nullptr, nullptr, nullptr);
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

	RCache.set_Element(s_lum_copy->E[3]);
	W = Device.fTimeDelta;
	RCache.set_c("adapt_params", W, H, rW, rH);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	// Resolve RT
	RContext->CopyResource(rt_LUM_Prev->pSurface, rt_LUM_D->pSurface);
}