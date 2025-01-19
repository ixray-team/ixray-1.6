//----------------------------------------------------
// file: stdafx.h
//----------------------------------------------------
#pragma once
// DirectX headers
#include <d3d11.h>
#include <d3d11shader.h>
#include "../xrEUI/stdafx.h"
#include "../../Layers/xrRenderDX9/xrD3DDefs.h"
#include "../../Layers/xrRender/r__types.h"

#include "../../utils/xrDXT/xrDXT.h"

#ifdef	XRECORE_EXPORTS
#define ECORE_API		__declspec(dllexport)
#else
#define ECORE_API		__declspec(dllimport)
#endif

#include "../../xrEngine/stdafx.h"
#include "../../xrEngine/device.h"
#include "../xrEProps/stdafx.h"
#include "../../xrCDB/xrCDB.h"
#include "../../xrSound/Sound.h"
#include "../../xrParticles/psystem.h"

#include "../../xrEngine/Fmesh.h"
#include "../../xrEngine/_d3d_extensions.h"
#include "..\Layers\xrRenderDX10\DxgiFormat.h"
#define smart_cast dynamic_cast

#ifndef O_SEQUENTIAL
#define O_SEQUENTIAL 0
#endif

#define DIRECTINPUT_VERSION 0x0800

#define         R_R1    1
#define         R_R2    2
#define         R_R4    4
#define         RENDER  R_R4
#define			REDITOR 1



#define PropertyGP(a,b)	__declspec( property( get=a, put=b ) )
#define THROW			FATAL("THROW");
#define THROW2(a)		R_ASSERT(a);
#define clMsg 			Msg

class PropValue;
class PropItem;

using PropItemVec = xr_vector<PropItem*>;
using PropItemIt = PropItemVec::iterator;

class ListItem;

using ListItemsVec = xr_vector<ListItem*>;

// some user components
using AnsiString = xr_string;
using AStringVec = xr_vector<AnsiString>;
using AStringIt = AStringVec::iterator;

using LPAStringVec = xr_vector<AnsiString*>;
using LPAStringIt = LPAStringVec::iterator;


#include "../Public/xrEProps.h"
#include "../../xrCore/log.h"
#include "Editor/ELog.h"
#include "../../xrEngine/defines.h"

#include "../../xrPhysics/xrPhysics.h"
#include "../../Layers/xrRender/FVF.h"

struct str_pred 
{
    IC bool operator()(LPCSTR x, LPCSTR y) const
    {	return strcmp(x,y)<0;	}
};
struct astr_pred
{
    IC bool operator()(const xr_string& x, const xr_string& y) const
    {	return x<y;	}
};

#include "Editor/device.h"
#include "../../xrEngine/Properties.h"
#include "Editor/render.h"
using FLvertexVec = xr_vector<FVF::L>;
using FLvertexIt = FLvertexVec::iterator;

using FTLvertexVec = xr_vector<FVF::TL>;
using FTLvertexIt = FTLvertexVec::iterator;

using FLITvertexVec = xr_vector<FVF::LIT>;
using FLITvertexIt = FLITvertexVec::iterator;

using RStrVec = xr_vector<shared_str>;
using RStrVecIt = RStrVec::iterator;

#include "Editor/EditorPreferences.h"

#ifdef _LEVEL_EDITOR                
	#include "../../xrCore/net_utils.h"
#endif

#define INI_NAME(buf) 		{FS.update_path(buf,"$app_data_root$",EFS.ChangeFileExt(UI->EditorName(),".ini").c_str());}
#define JSON_NAME(buf) 		{FS.update_path(buf,"$app_data_root$",EFS.ChangeFileExt(UI->EditorName(),".json").c_str());}
//#define INI_NAME(buf) 		{buf = buf+xr_string(Core.WorkingPath)+xr_string("\\")+EFS.ChangeFileExt(UI->EditorName(),".ini");}
#define DEFINE_INI(storage)	{string_path buf; INI_NAME(buf); storage->IniFileName=buf;}
#define NONE_CAPTION "<none>"
#define MULTIPLESEL_CAPTION "<multiple selection>"

// path definition
#define _server_root_		"$server_root$"
#define _server_data_root_	"$server_data_root$"
#define _local_root_		"$local_root$"
#define _import_			"$import$"
#define _sounds_			"$sounds$"
#define _textures_			"$textures$"
#define _objects_			"$objects$"
#define _maps_				"$maps$"
#define _groups_			"$groups$"
#define _temp_				"$temp$"
#define _omotion_			"$omotion$"
#define _omotions_			"$omotions$"
#define _smotion_			"$smotion$"
#define _detail_objects_	"$detail_objects$"

#define		TEX_POINT_ATT	"internal\\internal_light_attpoint"
#define		TEX_SPOT_ATT	"internal\\internal_light_attclip"

#include "../../Layers/xrRender/ETextureParams.h"
#include "../../Layers/xrRender/ResourceManager.h"

#include "../../Layers/xrRender/blenders/Blender_Recorder.h"
#include "../../Layers/xrRender/blenders/Blender.h"
#include "../../Layers/xrRender/blenders/Blender_CLSID.h"

#include "Editor/ImageManager.h"
inline xr_string ChangeFileExt(const char* name, const char* e)
{
	string_path path;
	xr_strcpy(path, name);
	if (strrchr(path,'.'))
	{
		strrchr(path, '.')[0] = 0;
	}
	xr_string str;
	str.append(path);
	str.append(e);
	return str;

}
inline xr_string ChangeFileExt(const xr_string&name, const char* e)
{
	string_path path;
	xr_strcpy(path, name.c_str());
	if (strrchr(path, '.'))
	{
		strrchr(path, '.')[0] = 0;
	}
	xr_string str;
	str.append(path);
	str.append(e);
	return str;

}
inline u32 TColor(u32 r)
{
	return r;
}

inline HRESULT DX11CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, DxgiFormat Format, UINT Pool, ID3D11Texture2D** ppTexture, HANDLE* pSharedHandle)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = Width;
	desc.Height = Height;
	desc.MipLevels = Levels;
	desc.Format = (DXGI_FORMAT)Format;
	//desc.Usage = (D3D11_USAGE)Usage;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.ArraySize = 1;
	desc.Usage = D3D_USAGE_DYNAMIC;
	desc.BindFlags = D3D_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D_CPU_ACCESS_WRITE;
	return RDevice->CreateTexture2D(&desc, NULL, ppTexture);
}

template<class T>
inline HRESULT DX11LockRect(
	T*		pTexture,
	UINT					Level,
	D3DLOCKED_RECT*			pLockedRect,
	const RECT*				pRect,
	DWORD					Flags
)
{
	D3D11_MAPPED_SUBRESOURCE sb;
	HRESULT hr = RContext->Map(pTexture, Level, D3D11_MAP_WRITE_DISCARD, 0, &sb);

	R_ASSERT(pLockedRect);
	pLockedRect->pBits = sb.pData;
	pLockedRect->Pitch = sb.RowPitch;
	
	return hr;
}

template<class T>
inline HRESULT DX11UnlockRect(
	T* pTexture,
	UINT Level
)
{
	RContext->Unmap(pTexture, Level);
	return S_OK;
}

inline HRESULT DX11Lock(
	ID3D11Resource* pResource,
	UINT  OffsetToLock,
	UINT  SizeToLock,
	void** ppbData,
	DWORD Flags
)
{
	D3D11_MAPPED_SUBRESOURCE sb;
	HRESULT hr = RContext->Map(pResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &sb);

	R_ASSERT(ppbData);
	*ppbData = sb.pData;
	return hr;
}

inline HRESULT DX11Unlock(ID3D11Resource* pResource)
{
	RContext->Unmap(pResource, 0);
	return S_OK;
}

#ifdef XRECORE_EXPORTS
inline void not_implemented()
{
	if (IsDebuggerPresent())
		DebugBreak();
	else
	{
		R_ASSERT(0);
	}
}
#endif
