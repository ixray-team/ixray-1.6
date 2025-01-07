#include "stdafx.h"
#pragma hdrstop

#include "DetailManager.h"

const u32	vs_size				= 3000;

void CDetailManager::soft_Load		()
{

	R_ASSERT(RCache.Vertex.Buffer());
	R_ASSERT(RCache.Index.Buffer());
	// Vertex Stream
	soft_Geom.create				(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, RCache.Vertex.Buffer(), RCache.Index.Buffer());
}

void CDetailManager::soft_Unload	()
{
	soft_Geom.destroy				();
}

void CDetailManager::soft_Render	()
{
	if (UseHW()) return;
	PROF_EVENT("CDetailManager::soft_Render")
	RCache.set_CullMode		(CULL_NONE);
	RCache.set_xform_world	(Fidentity);
	// Render itself
	// float	fPhaseRange	= PI/16;
	// float	fPhaseX		= _sin(RDEVICE.fTimeGlobal*0.1f)	*fPhaseRange;
	// float	fPhaseZ		= _sin(RDEVICE.fTimeGlobal*0.11f)*fPhaseRange;
	u32 C = 0xFFFFFFFF;

#ifdef _EDITOR
	if (psDeviceFlags.test(rsEnvironment))
	{
		Fvector sun = g_pGamePersistent->Environment().CurrentEnv->sun_color;
		sun.mul(0.5f);
		C = color_rgba_f(sun.x, sun.y, sun.z, 1.f);
	}
#endif

	// Get index-stream
	_IndexStream&	_IS		= RCache.Index;
	_VertexStream&	_VS		= RCache.Vertex;
#ifndef _EDITOR 
	for (CDetail& Object : objects)
	{
#else
	for (CDetail* D : objects)
	{
		CDetail& Object = *D;
#endif
		u32	vCount_Object	= Object.number_vertices;
		u32	iCount_Object	= Object.number_indices;

		auto& items = Object.m_items[0][0];
		if(items.empty()) continue;
		u32	vCount_Total		= (u32)items.size()*vCount_Object;
		// calculate lock count needed
		u32	lock_count			= vCount_Total/vs_size;
		if	(vCount_Total>(lock_count*vs_size))	lock_count++;

		// calculate objects per lock
		u32	o_total			= (u32)items.size();
		u32	o_per_lock		= o_total/lock_count;
		if  (o_total > (o_per_lock*lock_count))	o_per_lock++;

		// Fill VB (and flush it as nesessary)
		RCache.set_Shader	(Object.shader);

		Fmatrix		mXform;
		for (u32 L_ID=0; L_ID<lock_count; L_ID++)
		{
			// Calculate params
			u32	item_start	= L_ID*o_per_lock;
			u32	item_end	= item_start+o_per_lock;
			if (item_end>o_total)	item_end = o_total;
			if (item_end<=item_start)	break;
			u32	item_range	= item_end-item_start;

			// Calc Lock params
			u32	vCount_Lock	= item_range*vCount_Object;
			u32	iCount_Lock = item_range*iCount_Object;

			// Lock buffers
			u32	vBase,iBase,iOffset=0;
			CDetail::fvfVertexOut* vDest	= (CDetail::fvfVertexOut*)	_VS.Lock(vCount_Lock,soft_Geom->vb_stride,vBase);
			u16*	iDest					= (u16*)					_IS.Lock(iCount_Lock,iBase);

			// Filling itself
		    for (u32 item_idx=item_start; item_idx<item_end; ++item_idx)
			{
				CDetail::SlotItem& Instance = *items.at(item_idx);

				// Build matrix
				Fmatrix& M = Instance.mRotY_calculated;
				mXform._11=M._11; mXform._12=M._12;	mXform._13=M._13; mXform._14=M._14;
				mXform._21=M._21; mXform._22=M._22;	mXform._23=M._23; mXform._24=M._24;
				mXform._31=M._31; mXform._32=M._32;	mXform._33=M._33; mXform._34=M._34;
				mXform._41=M._41; mXform._42=M._42; mXform._43=M._43; mXform._44=1;

				// Transfer vertices
				{
					CDetail::fvfVertexIn	*srcIt = Object.vertices, *srcEnd = Object.vertices+Object.number_vertices;
					CDetail::fvfVertexOut	*dstIt = vDest;

					for	(; srcIt!=srcEnd; srcIt++, dstIt++){
		                mXform.transform_tiny	(dstIt->P,srcIt->P);
		                dstIt->C	= C;
		                dstIt->u	= srcIt->u;
		                dstIt->v	= srcIt->v;
					}
				}

				// Transfer indices (in 32bit lines)
				VERIFY	(iOffset<65535);
				{
					u32	item	= (iOffset<<16) | iOffset;
					u32	count	= Object.number_indices/2;
					LPDWORD	sit		= LPDWORD(Object.indices);
					LPDWORD	send	= sit+count;
					LPDWORD	dit		= LPDWORD(iDest);
					for		(; sit!=send; dit++,sit++)	*dit=*sit+item;
					if		(Object.number_indices&1)
						iDest[Object.number_indices-1]=(u16)(Object.indices[Object.number_indices-1]+u16(iOffset));
				}

				// Increment counters
				vDest					+=	vCount_Object;
				iDest					+=	iCount_Object;
				iOffset					+=	vCount_Object;
			}
			_VS.Unlock		(vCount_Lock,soft_Geom->vb_stride);
			_IS.Unlock		(iCount_Lock);

			// Render
			u32	dwNumPrimitives		= iCount_Lock/3;
			RCache.set_Geometry		(soft_Geom);
			RCache.Render			(D3DPT_TRIANGLELIST,vBase,0,vCount_Lock,iBase,dwNumPrimitives);
		}
	}

	RCache.set_CullMode		(CULL_CCW);
}