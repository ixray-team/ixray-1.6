////////////////////////////////////////////////////////////////////////////
//	Module 		: xrGame.cpp
//	Created 	: 07.01.2001
//  Modified 	: 27.05.2004
//	Author		: Aleksandr Maksimchuk and Oles' Shyshkovtsov
//	Description : Defines the entry point for the DLL application.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "object_factory.h"
#include "ui/xrUIXmlParser.h"
#include "../xrEngine/xr_level_controller.h"
extern void CCC_RegisterCommands();

#ifdef NDEBUG
//namespace std {
//	void terminate()
//	{
//		abort();
//	}
//}
#endif // #ifdef NDEBUG

extern "C" {
	DLL_API void __cdecl xrGameInitialize()
	{
		// register console commands
		CCC_RegisterCommands();
		// keyboard binding
		CCC_RegisterInput			();

	}
	DLL_API DLL_Pure*	__cdecl xrFactory_Create		(CLASS_ID clsid)
	{
		DLL_Pure			*object = object_factory().client_object(clsid);
#ifdef DEBUG
		if (!object)
			return			(0);
#endif
		object->CLS_ID		= clsid;
		return				(object);
	}

	DLL_API void		__cdecl	xrFactory_Destroy		(DLL_Pure* O)
	{
		xr_delete			(O);
	}
};


