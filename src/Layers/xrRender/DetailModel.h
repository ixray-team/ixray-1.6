#ifndef DetailModelH
#define DetailModelH
#pragma once

#include "IRenderDetailModel.h"

class ECORE_API CDetail		: public IRender_DetailModel
{
public:
	struct	SlotItem
	{								// один кустик
		float						scale;
		Fmatrix						mRotY;
		Fmatrix						mRotY_calculated;
		u32							vis_ID;				// индекс в visibility списке он же тип [не качается, качается1, качается2]
		float						c_hemi;
#if RENDER==R_R1
		float						c_sun;
		Fvector						c_rgb;
#endif
	};
	xr_vector<xr_shared_ptr<SlotItem>> m_items[3][2];
	void			Load		(IReader* S);
	void			Optimize	();
	virtual void	Unload		();

	virtual void	transfer	(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset);
	virtual void	transfer	(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset, float du, float dv);
	virtual			~CDetail	();
};
#endif
