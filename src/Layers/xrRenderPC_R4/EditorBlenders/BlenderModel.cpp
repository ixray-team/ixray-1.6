#include "stdafx.h"
#pragma hdrstop

#include "../../xrRenderPC_R1/Blender_Model.h"
#include "../../xrRender/uber_deffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlender_Model::CBlender_Model()
{
	description.CLS = B_MODEL;
	description.version = 2;
	oTessellation.Count = 4;
	oTessellation.IDselected = 0;
	oAREF.value = 32;
	oAREF.min = 0;
	oAREF.max = 255;
	oBlend.value = FALSE;
}

CBlender_Model::~CBlender_Model()
{

}

void	CBlender_Model::Save(IWriter& fs)
{
	IBlender::Save(fs);
	xrPWRITE_PROP(fs, "Use alpha-channel", xrPID_BOOL, oBlend);
	xrPWRITE_PROP(fs, "Alpha ref", xrPID_INTEGER, oAREF);
	xrP_TOKEN::Item	I;
	xrPWRITE_PROP(fs, "Tessellation", xrPID_TOKEN, oTessellation);
	I.ID = 0; xr_strcpy(I.str, "NO_TESS");	fs.w(&I, sizeof(I));
	I.ID = 1; xr_strcpy(I.str, "TESS_PN");	fs.w(&I, sizeof(I));
	I.ID = 2; xr_strcpy(I.str, "TESS_HM");	fs.w(&I, sizeof(I));
	I.ID = 3; xr_strcpy(I.str, "TESS_PN+HM");	fs.w(&I, sizeof(I));
}

void	CBlender_Model::Load(IReader& fs, u16 version)
{
	IBlender::Load(fs, version);

	switch (version)
	{
	case 0:
		oAREF.value = 32;
		oAREF.min = 0;
		oAREF.max = 255;
		oBlend.value = FALSE;
		break;
	case 1:
	default:
		xrPREAD_PROP(fs, xrPID_BOOL, oBlend);
		xrPREAD_PROP(fs, xrPID_INTEGER, oAREF);
		break;
	}
	if (version > 1)
	{
		xrPREAD_PROP(fs, xrPID_TOKEN, oTessellation);
	}
}

void CBlender_Model::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	bool is_blend = oBlend.value && oAREF.value < 16;

	if (is_blend) {
		RImplementation.addShaderOption("FORWARD_ONLY", "1");
	}

	uber_deffer(C, true, "deffer_model", "deffer_base", !is_blend && !!oBlend.value, 0, true);

	if (is_blend) {
		C.PassSET_ZB(TRUE, FALSE);
		C.PassSET_Blend(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, true, 0);
	}

	C.r_End();
}