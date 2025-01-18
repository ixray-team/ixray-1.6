// Blender_Vertex_aref.cpp: implementation of the CBlender_Vertex_aref class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#include "../../xrRenderPC_R1/Blender_Vertex_aref.h"
#include "../../xrRender/uber_deffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlender_Vertex_aref::CBlender_Vertex_aref()
{
	description.CLS		= B_VERT_AREF;
	description.version	= 1;
	oAREF.value			= 32;
	oAREF.min			= 0;
	oAREF.max			= 255;
	oBlend.value		= FALSE;
}

CBlender_Vertex_aref::~CBlender_Vertex_aref()
{

}

void CBlender_Vertex_aref::Save(IWriter& fs)
{
	IBlender::Save(fs);
	xrPWRITE_PROP(fs, "Alpha ref", xrPID_INTEGER, oAREF);
	xrPWRITE_PROP(fs, "Alpha-blend", xrPID_BOOL, oBlend);
}

void CBlender_Vertex_aref::Load(IReader& fs, u16 version)
{
	IBlender::Load(fs, version);

	switch (version)
	{
	case 0:
		xrPREAD_PROP(fs, xrPID_INTEGER, oAREF);
		oBlend.value = FALSE;
		break;
	case 1:
	default:
		xrPREAD_PROP(fs, xrPID_INTEGER, oAREF);
		xrPREAD_PROP(fs, xrPID_BOOL, oBlend);
		break;
	}
}

void CBlender_Vertex_aref::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	if (!!oBlend.value) 
	{
		RImplementation.addShaderOption("FORWARD_ONLY", "1");
	}

	uber_deffer(C, true, "deffer_base", "deffer_base", !oBlend.value, 0, true);

	if (!!oBlend.value)
	{
		C.PassSET_Blend(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, true, 0);
	}

	C.r_End();
}