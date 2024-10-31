/*
In r4_rendertarget.h add:

	ID3DTexture2D* miptest_tex;
	ref_texture miptest_access;
	ID3DRenderTargetView* miptest_rtv;
	ID3DShaderResourceView* miptest_srv;
	void phase_miptest();

Create new blender, etc.

Then, you can call the mipchain generator with:
	phase_miptest();
	
The output will be stored in $user$cock, that means you will bind it in your shader with:
	C.r_dx10Texture("t_hizbuffer", "$user$cock");
	
Second (1) element of your s_hizbuffer blender cannot contain any textures.
We bind mipped SRV directly (Fuck Your XRay Engine).

This is my last serious contribution to this piece of shit engine.
Cheers.
*/

#include "stdafx.h"
//#include "r4_rendertarget.h"

void CRenderTarget::phase_miptest()
{
	u32 Offset = 0;
	u32 vertex_color = color_rgba(0, 0, 0, 255);

	//viewport
    D3D_VIEWPORT VP = {
        0.0f,
        0.0f,
        RCache.get_width(),
        RCache.get_height(),
        0.0f, 
        1.0f
    };

	//fill vb
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	//consts
	int mip_width = RCache.get_width();
	int mip_height = RCache.get_height();
	int mip_count = 6;

	//create our mipped texture
	D3D_TEXTURE2D_DESC descTex {};
	descTex.Format = DXGI_FORMAT_R16G16_FLOAT;
	descTex.Width = mip_width;
	descTex.Height = mip_height;
	descTex.MipLevels = mip_count;
	descTex.ArraySize = 1;
	descTex.SampleDesc.Count = 1;
	descTex.SampleDesc.Quality = 0;	
	descTex.Usage = D3D11_USAGE_DEFAULT;
	descTex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	descTex.CPUAccessFlags = 0;
	R_CHK(RDevice->CreateTexture2D(&descTex, NULL, &miptest_tex));


	//fill top mip with some data...
	{
		//create rtv for mip0
		D3D_RENDER_TARGET_VIEW_DESC descRTV {};
		descRTV.Format = descTex.Format;
		descRTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		descRTV.Texture2D.MipSlice = 0;
		RDevice->CreateRenderTargetView(miptest_tex, &descRTV, &miptest_rtv);
	
		//bind rtv directly
		RCache.set_RT(miptest_rtv, 0);

		//set everything in place, render
		RCache.set_Element(s_mipchain->E[0]);
		RCache.set_Geometry(g_combine);
		RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

		//release rtv, unbind 0 slot
		miptest_rtv->Release();	
		RCache.set_RT(NULL, 0);
	}

	//create mipchain
	for (int i = 1; i < mip_count; i++)
	{
		//Halving the vp dimensions each mip
		mip_width /= 2;
		mip_height /= 2;

		//create rtv
		D3D_RENDER_TARGET_VIEW_DESC descRTV { };
		descRTV.Format = descTex.Format;
		descRTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		descRTV.Texture2D.MipSlice = i;
		RDevice->CreateRenderTargetView(miptest_tex, &descRTV, &miptest_rtv);

		//create srv
		D3D_SHADER_RESOURCE_VIEW_DESC descSRV { };
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		descSRV.Texture2D.MostDetailedMip = i - 1; //prev mip
		descSRV.Texture2D.MipLevels = 1;
		RDevice->CreateShaderResourceView(miptest_tex, &descSRV, &miptest_srv);

		//set the viewport
		VP.Width = float(mip_width);
		VP.Height = float(mip_height);		
		RContext->RSSetViewports(1, &VP);

		//bind rtv (directly)
		RCache.set_RT(miptest_rtv, 0);

		//set ps, vs, states
		RCache.set_Element(s_mipchain->E[1]);

		//important bit: BIND FUCKING SRV TO SLOT 0. IF IT BREAKS SOMETHING - FUCK YOU AND YOUR XRAY ENGINE.
		SRVSManager.SetPSResource(0, miptest_srv);

		//set geometry, and draw the triangle
		RCache.set_Geometry(g_combine);
		RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

		//release rtv and srv
		miptest_rtv->Release();
		miptest_srv->Release();
	}

	//create 'xray srv' so we can actually sample the shit in other shaders. fuck you and your xray engine 2.
	miptest_access.create("$user$cock");
	miptest_access->surface_set(miptest_tex);

	//release the tex resource. to avoid a fucking leaks
	miptest_tex->Release();

	//restore viewport
	VP.Width = float(RCache.get_width());
	VP.Height = float(RCache.get_height());		
	RContext->RSSetViewports(1, &VP);

	//jebac xraya
}