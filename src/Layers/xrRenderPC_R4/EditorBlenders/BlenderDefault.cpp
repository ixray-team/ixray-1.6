// BlenderDefault.cpp: implementation of the CBlender_default class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#include "../../xrRenderPC_R1/BlenderDefault.h"
#include "../../xrRender/uber_deffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlender_default::CBlender_default	()
{
	description.CLS		        = B_DEFAULT;
	description.version         = 1;
	oTessellation.Count         = 4;
	oTessellation.IDselected	= 0;
}

CBlender_default::~CBlender_default	()
{

}

void CBlender_default::Save(IWriter& fs)
{
	IBlender::Save(fs);
	xrP_TOKEN::Item	I;
	xrPWRITE_PROP(fs, "Tessellation", xrPID_TOKEN, oTessellation);

	I.ID = 0; xr_strcpy(I.str,"NO_TESS");		fs.w(&I,sizeof(I));
	I.ID = 1; xr_strcpy(I.str,"TESS_PN");		fs.w(&I,sizeof(I));
	I.ID = 2; xr_strcpy(I.str,"TESS_HM");		fs.w(&I,sizeof(I));
	I.ID = 3; xr_strcpy(I.str,"TESS_PN+HM");	fs.w(&I,sizeof(I));
}

void CBlender_default::Load(IReader& fs, u16 version)
{
	IBlender::Load(fs, version);
	if (version > 0)
	{
		xrPREAD_PROP(fs, xrPID_TOKEN, oTessellation);
	}
}

void CBlender_default::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	uber_deffer(C, true, "deffer_base", "deffer_base", false, 0, true);
	C.r_End();
}