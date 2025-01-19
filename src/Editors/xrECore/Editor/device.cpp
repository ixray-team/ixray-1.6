//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop
#include "../xrEngine/GameFont.h"
#include <sal.h>
#include "ImageManager.h"
#include "ui_main.h"
#include "render.h"
#include "../Engine/XrGameMaterialLibraryEditors.h"
#include "../Layers/xrRender/ResourceManager.h"
#include "../Layers/xrRender/dxRenderDeviceRender.h"
#include "../Layers/xrRenderDX10/dx10BufferUtils.h"
#include "../Layers/xrRenderDX10/StateManager/dx10StateManager.h"
#include "UI_ToolsCustom.h"

CEditorRenderDevice 	*	EDevice;
bool g_bIsEditor;

extern int	rsDVB_Size;
extern int	rsDIB_Size;

//CStatsPhysics* _BCL			CEditorRenderDevice::StatPhysics() { return Statistic; }
void	   _BCL			CEditorRenderDevice::AddSeqFrame(pureFrame* f, bool mt) { seqFrame.Add(f, REG_PRIORITY_LOW); }
void	   _BCL			CEditorRenderDevice::RemoveSeqFrame(pureFrame* f) { seqFrame.Remove(f); }

ENGINE_API xr_atomic_bool g_bRendering;
//---------------------------------------------------------------------------
#include <luabind/luabind.hpp>

static LPVOID __cdecl luabind_allocator(
	luabind::memory_allocation_function_parameter const,
	void const* const pointer,
	size_t const size
)
{
	if (!size)
	{
		LPVOID	non_const_pointer = const_cast<LPVOID>(pointer);
		xr_free(non_const_pointer);
		return	(0);
	}

	if (!pointer)
	{
		return	(Memory.mem_alloc(size));
	}

	LPVOID non_const_pointer = const_cast<LPVOID>(pointer);
	return (Memory.mem_realloc(non_const_pointer, size));
}

void setup_luabind_allocator()
{
	luabind::allocator = &luabind_allocator;
	luabind::allocator_parameter = 0;
}


CEditorRenderDevice::CEditorRenderDevice()
{
	RadiusRender = 400;
	psDeviceFlags.assign(rsStatistic|rsFilterLinear|rsFog|rsDrawGrid);
// dynamic buffer size
	rsDVB_Size		= 1024 * 16;
	rsDIB_Size		= 2048;
// default initialization
    m_ScreenQuality = 1.f;
	//dwMaximized = 0;
    TargetWidth 		= TargetHeight 	= 256;
	dwRealWidth = dwRealHeight = 256;
	mProject.identity();
    mFullTransform.identity();
    mView.identity	();
	m_WireShader	= 0;
	m_SelectionShader = 0;

    b_is_Ready 			= FALSE;
	b_is_Active			= FALSE;

	// Engine flow-control
	fTimeDelta		= 0;
	fTimeGlobal		= 0;
	dwTimeDelta		= 0;
	dwTimeGlobal	= 0;

	dwFillMode		= D3DFILL_SOLID;
    dwShadeMode		= D3DSHADE_GOURAUD;

    m_CurrentShader	= 0;
    //pSystemFont		= 0;

	m_MaterialBuffer	= 0;
	m_LightBuffer		= 0;

	fASPECT 		= 1.f;
	fFOV 			= 60.f;
    dwPrecacheFrame = 0;
	GameMaterialLibraryEditors = new XrGameMaterialLibraryEditors();
	PGMLib = GameMaterialLibraryEditors;

	DevicePtr = this;
	g_bIsEditor = true;

	setup_luabind_allocator();
}

CEditorRenderDevice::~CEditorRenderDevice()
{
	VERIFY(!b_is_Ready);
	GameMaterialLibraryEditors = nullptr;
}

#include "../../../xrCore/API/xrAPI.h"
#include "../../../Layers/xrRender/dxRenderFactory.h"
#include "../../../Layers/xrRender/dxUIRender.h"
#include "../../../Layers/xrRender/dxDebugRender.h"
#include "../xrCore/appinfo.h"

typedef void __cdecl ttapi_Done_func(void);

void CEditorRenderDevice::Initialize()
{
    m_DefaultMat.set(1,1,1);

	RenderFactory = &RenderFactoryImpl;
	UIRender = &UIRenderImpl;

#ifdef DEBUG_DRAW
	DRender = &DebugRenderImpl;
#endif

	SDL_Init(0);

	// compiler shader
    string_path fn;
    FS.update_path(fn,_game_data_,"shaders_xrlc.xr");
    if (FS.exist(fn)){
    	ShaderXRLC.Load(fn);
    }else{
    	ELog.DlgMsg(mtInformation,"Can't find file '%s'",fn);
    }
	CreateWindow();


	// Startup shaders
	Create();

    ::RImplementation.Initialize();
	UIRenderImpl.CreateUIGeom();

	Resize(EPrefs->start_w, EPrefs->start_h, EPrefs->start_maximized);

	SDL_GetWindowSizeInPixels(g_AppInfo.Window, &Width, &Height);
	SDL_GetWindowPosition(g_AppInfo.Window, &PosX, &PosY);

	if (EPrefs->start_maximized)
		SDL_MaximizeWindow(g_AppInfo.Window);
	
	SDL_ShowWindow(g_AppInfo.Window);
}

void CEditorRenderDevice::ShutDown()
{
	UIRenderImpl.DestroyUIGeom();
	::RImplementation.ShutDown	();

	ShaderXRLC.Unload	();

	// destroy context
	Destroy				();
}

void CEditorRenderDevice::InitTimer()
{
	Timer_MM_Delta = 0;
	{
		u32 time_mm = clock();
		while (clock() == time_mm);			// wait for next tick
		u32 time_system = clock();
		u32 time_local = TimerAsync();
		Timer_MM_Delta = time_system - time_local;
	}
}

void CEditorRenderDevice::Clear()
{
	Fcolor ClearColor = Fcolor(0.0f, 0.0f, 0.0f);

	if (EPrefs)
	{
		float color[4] = 
		{ 
			color_get_R(EPrefs->scene_clear_color) / 255.f, 
			color_get_G(EPrefs->scene_clear_color) / 255.f,
			color_get_B(EPrefs->scene_clear_color) / 255.f, 
			1
		};
		ClearColor = Fcolor(color[0], color[1], color[2], color[3]);
	}

	//RContext->ClearRenderTargetView( RTarget, (float*)&ClearColor );
	//RContext->ClearRenderTargetView(RCache.get_RT(0), (float*)&ClearColor);
	RContext->ClearDepthStencilView( RDepth, D3D_CLEAR_DEPTH | D3D_CLEAR_STENCIL, 1.0f, 0 );

	if (RCache.get_RT())
	{
		RContext->ClearRenderTargetView(RCache.get_RT(0), (float*)&ClearColor);
	}
	if (RCache.get_ZB())
	{
		RContext->ClearDepthStencilView(RCache.get_ZB(), D3D_CLEAR_DEPTH | D3D_CLEAR_STENCIL, 1.0f, 0);
	}
}

//---------------------------------------------------------------------------
void CEditorRenderDevice::RenderNearer(float n){
    mProject._43=m_fNearer-n;
    RCache.set_xform_project(mProject);
}
void CEditorRenderDevice::ResetNearer(){
    mProject._43=m_fNearer;
    RCache.set_xform_project(mProject);
}
//---------------------------------------------------------------------------
bool CEditorRenderDevice::Create()
{
	if (b_is_Ready)	return false;

	//Statistic = EStatistic;
	ELog.Msg(mtInformation,"Starting RENDER device...");


	//HW.CreateDevice		(m_hWnd, true);
	if (UI)
	{
		hwnd = (HWND)SDL_GetProperty(SDL_GetWindowProperties(g_AppInfo.Window), "SDL.window.win32.hwnd", nullptr);
		string_path 		ini_path;
		string_path			ini_name;
		xr_strcpy			(ini_name, UI->EditorName());
		xr_strcat			(ini_name, "_imgui.ini");
		FS.update_path(ini_path, "$app_data_root$", ini_name);
		
		if (!FS.exist(ini_path))
			UI->ResetUI();
		
		InitRenderDeviceEditor();
		UI->Initialize(hwnd, RDevice, RContext, ini_path);
	}
	
	// after creation
	dwFrame				= 0;

	string_path 		sh;
    FS.update_path		(sh,_game_data_,"shaders.xr");

    IReader* F			= 0;
	if (FS.exist(sh))
		F				= FS.r_open(0,sh);
	Resources			= new CResourceManager	();

    // if build options - load textures immediately
    if (strstr(Core.Params,"-build")||strstr(Core.Params,"-ebuild"))
        EDevice->Resources->DeferredLoad(FALSE);

	g_FontManager = new CFontManager();

    _Create				(F);
	FS.r_close			(F);

	::Render->create();

	g_FontManager->InitializeFonts();

	Statistic = new CEStats();

	ELog.Msg			(mtInformation, "D3D: initialized");

	return true;
}

//---------------------------------------------------------------------------
void CEditorRenderDevice::Destroy()
{
	if (!b_is_Ready) return;
	ELog.Msg(mtInformation, "Destroying Direct3D...");

	//HW.Validate			();
	::Render->destroy();

	// before destroy
	_Destroy(FALSE);

	xr_delete(Resources);

	UI->Destroy();
	// real destroy
	//HW.DestroyDevice	();

	ELog.Msg(mtInformation, "D3D: device cleared");
	// xr_delete			(Statistic);
}

//---------------------------------------------------------------------------
void CEditorRenderDevice::_SetupStates()
{
	return;
	//Caps.Update();
	for (u32 i=0; i<Caps.raster.dwStages; i++){
		float fBias = -1.f;
		SetSS( i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD) (&fBias)));
	}

	EDevice->SetRS(D3DRS_DITHERENABLE,	TRUE				);
    EDevice->SetRS(D3DRS_COLORVERTEX,		TRUE				);
    EDevice->SetRS(D3DRS_STENCILENABLE,	FALSE				);
    EDevice->SetRS(D3DRS_ZENABLE,			TRUE				);
    EDevice->SetRS(D3DRS_SHADEMODE,		D3DSHADE_GOURAUD	);
	EDevice->SetRS(D3DRS_CULLMODE,		D3DCULL_CCW			);
	EDevice->SetRS(D3DRS_ALPHAFUNC,		D3DCMP_GREATER		);
	EDevice->SetRS(D3DRS_LOCALVIEWER,		TRUE				);
    EDevice->SetRS(D3DRS_NORMALIZENORMALS,TRUE				);

	EDevice->SetRS(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	EDevice->SetRS(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL);
	EDevice->SetRS(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	EDevice->SetRS(D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_COLOR1	);

    ResetMaterial();
}
//---------------------------------------------------------------------------
void CEditorRenderDevice::_Create(IReader* F)
{
	b_is_Ready				= TRUE;

	// General Render States
    _SetupStates		();
    
    RCache.OnDeviceCreate		();
	Resources->OnDeviceCreate	(F);
	::RImplementation.OnDeviceCreate	();

    m_WireShader.create			("editor\\wire");
	m_WireShaderAxis.create		("editor_wire_axis");
    m_SelectionShader.create	("editor\\selection");

	//dx10BufferUtils::CreateConstantBuffer( &m_MaterialBuffer, sizeof( Fmaterial ) );
	//dx10BufferUtils::CreateConstantBuffer( &m_LightBuffer, sizeof( Flight ) * MAX_EDITOR_LIGHT );

	texture_null.create("ed\\ed_nodata");
	texture_null->Load();
	UIChooseForm::SetNullTexture((ID3D11Texture2D*)texture_null->pSurface);

	// signal another objects
    UI->OnDeviceCreate			();       

	EDevice->InitWindowStyle();
}

void CEditorRenderDevice::_Destroy(BOOL	bKeepTextures)
{
	b_is_Ready 						= FALSE;
    m_CurrentShader				= 0;

	if (m_LightBuffer)
	{
		m_LightBuffer->Release();
		m_LightBuffer = 0;

		m_MaterialBuffer->Release();
		m_MaterialBuffer = 0;
	}

    UI->OnDeviceDestroy			();

	m_WireShader.destroy		();
	m_WireShaderAxis.destroy	();
	m_SelectionShader.destroy	();
	texture_null.destroy		();

	::RImplementation.Models->OnDeviceDestroy	();

	Resources->OnDeviceDestroy	(bKeepTextures);

	RCache.OnDeviceDestroy		();
	::RImplementation.OnDeviceDestroy	();
}

//---------------------------------------------------------------------------
void  CEditorRenderDevice::Resize(int w, int h, bool maximized)
{
	m_RenderArea = w * h;

	dwRealWidth = w;
	dwRealHeight = h;

	Reset(false);
	UI->RedrawScene();
}

void CEditorRenderDevice::Reset(bool)
{
	u32 tm_start = TimerAsync();

	Resources->reset_begin();
	Resources->DeferredUnload();
	UI->ResetBegin();

	Memory.mem_compact();
	ResizeBuffers(dwRealWidth, dwRealHeight);
	SDL_SetWindowSize(g_AppInfo.Window, dwRealWidth, dwRealHeight);

	Resources->reset_end();
	Resources->DeferredUpload();

	UI->ResetEnd(RDevice, RContext);
	_SetupStates();

	UIChooseForm::SetNullTexture((ID3D11Texture2D*)texture_null->pSurface);

	u32 tm_end = TimerAsync();
	Msg("*** RESET [%d ms]", tm_end - tm_start);
}

void CEditorRenderDevice::Reset(IReader* F, BOOL bKeepTextures)
{
	CTimer tm;
	tm.Start();
	_Destroy(bKeepTextures);
	_Create(F);
	Msg("*** RESET [%d ms]", tm.GetElapsed_ms());
}

void CEditorRenderDevice::MaximizedWindow()
{
	if (IsZoomed(hwnd))
		SendMessageW(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);

	//
	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~WS_THICKFRAME;
	SetWindowLong(hwnd, GWL_STYLE, style);

	GetWindowRect(hwnd, &EDevice->NormalWinSize);
	EDevice->NormalWinSizeSaved = true;
	EDevice->isZoomed = true;

	auto CurrentMonitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(CurrentMonitor, &minfo);

	const RECT& workArea = minfo.rcWork;

	SDL_SetWindowSize(g_AppInfo.Window, workArea.right - workArea.left, workArea.bottom - workArea.top);
	SDL_SetWindowPosition(g_AppInfo.Window, workArea.left, workArea.top);
}

void CEditorRenderDevice::ResoreWindow(bool moving)
{
	if (EDevice->NormalWinSizeSaved)
	{
		auto& r = EDevice->NormalWinSize;
		MoveWindow(hwnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);

		if (moving)
		{
			float mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);
			SDL_SetWindowPosition(g_AppInfo.Window, mouseX / 2, mouseY - 10);
		}
	}
	{
		LONG style = GetWindowLong(hwnd, GWL_STYLE);
		style |= WS_THICKFRAME;
		SetWindowLong(hwnd, GWL_STYLE, style);
	}
	EDevice->isZoomed = false;


}

bool CEditorRenderDevice::Begin()
{
	VERIFY(b_is_Ready);
	mFullTransform_saved = mFullTransform;
	mProject_saved = mProject;
	mView_saved = mView;
	vCameraPosition_saved = vCameraPosition;
	//HW.Validate		();

	VERIFY(FALSE == g_bRendering);

	Clear();

	RCache.OnFrameBegin();
	g_bRendering = TRUE;
	return		TRUE;
}

//---------------------------------------------------------------------------
void CEditorRenderDevice::End()
{
	VERIFY(b_is_Ready);
	g_bRendering = 	FALSE;
	// end scene
	RCache.OnFrameEnd();

	RSwapchain->Present(0, 0);
}

void CEditorRenderDevice::UpdateMaterial()
{
	R_ASSERT( m_MaterialBuffer );
	RContext->UpdateSubresource( m_MaterialBuffer, 0, NULL, &m_CurrentMat, 0, 0 );
}

void CEditorRenderDevice::UpdateLight()
{
	R_ASSERT( m_LightBuffer );
	RContext->UpdateSubresource( m_LightBuffer, 0, NULL, m_Lights, 0, 0 );
}

void CEditorRenderDevice::UpdateView()
{
// set camera matrix
	if (!Tools->UpdateCamera())
	{
		UI->CurrentView().m_Camera.GetView(mView);
	}
    RCache.set_xform_view(mView);
    mFullTransform.mul(mProject,mView);

// frustum culling sets
    ::Render->ViewBase.CreateFromMatrix(mFullTransform,FRUSTUM_P_ALL);
}

void CEditorRenderDevice::FrameMove()
{
	dwFrame++;

	// Timer
    float fPreviousFrameTime = Timer.GetElapsed_sec(); Timer.Start();	// previous frame
    fTimeDelta = 0.1f * fTimeDelta + 0.9f*fPreviousFrameTime;			// smooth random system activity - worst case ~7% error
    if (fTimeDelta>.1f) fTimeDelta=.1f;									// limit to 15fps minimum

    fTimeGlobal		= TimerGlobal.GetElapsed_sec(); //float(qTime)*CPU::cycles2seconds;
    dwTimeGlobal	= TimerGlobal.GetElapsed_ms	();	//u32((qTime*u64(1000))/CPU::cycles_per_second);
    dwTimeDelta		= iFloor(fTimeDelta*1000.f+0.5f);
    dwTimeContinual	= dwTimeGlobal;

	if (!Tools->UpdateCamera())
	{
		UI->CurrentView().m_Camera.Update(fTimeDelta);
	}

    // process objects
	seqFrame.Process(rp_Frame);
}

void CEditorRenderDevice::InitWindowStyle()
{
	UI->InitWindowIcons();

#if _WINDOWS
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	style &= ~WS_CAPTION;
	SetWindowLongPtr(hwnd, GWL_STYLE, style);
#else
	SDL_SetWindowResizable(g_AppInfo.Window, SDL_TRUE);
	SDL_SetWindowHitTest(g_AppInfo.Window, HitTestCallback, 0);
#endif
}

void CEditorRenderDevice::DP(D3DPRIMITIVETYPE pt, ref_geom geom, u32 vBase, u32 pc)
{
	ref_shader S 			= m_CurrentShader?m_CurrentShader:m_WireShader;
    u32 dwRequired			= S->E[0]->passes.size();
    for (u32 dwPass = 0; dwPass<dwRequired; dwPass++){
    	RCache.set_Shader	(S,dwPass);
		RCache.set_Geometry(geom);
		RCache.Render		(pt,vBase,pc);
    }
}

void CEditorRenderDevice::DIP(D3DPRIMITIVETYPE pt, ref_geom geom, u32 baseV, u32 startV, u32 countV, u32 startI, u32 PC)
{
	ref_shader S 			= m_CurrentShader?m_CurrentShader:m_WireShader;
    u32 dwRequired			= S->E[0]->passes.size();
    RCache.set_Geometry		(geom);
    for (u32 dwPass = 0; dwPass<dwRequired; dwPass++){
    	RCache.set_Shader	(S,dwPass);
		RCache.Render		(pt,baseV,startV,countV,startI,PC);
    }
}

static CSimulator g_Simulator;

void CEditorRenderDevice::SetRS(D3DRENDERSTATETYPE p1, u32 p2)
{
	VERIFY(b_is_Ready);

	static u32 stencilfunc = 0;
	static u32 stencilref = 0;
	static u32 stencilmask = 0;
	static u32 stencilwritemask = 0;
	static u32 stencilfail = 0;
	static u32 stencilpass = 0;
	static u32 stencilzfail = 0;

	switch (p1)
	{
	case D3DRS_ZENABLE: 
		RCache.set_Z(p2);
		break;    /*case D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
	case D3DRS_FILLMODE: 
		StateManager.SetFillMode(p2);
		break;    /*case D3DFILLMODE */
	case D3DRS_SHADEMODE: 
		R_ASSERT(0);
		break;    /*case D3DSHADEMODE */
	case D3DRS_ZWRITEENABLE: 
		RCache.set_ZWrite(p2);
		break;   /* TRUE to enable z writes */
	case D3DRS_ALPHATESTENABLE: 
		RCache.set_AlphaRef(p2);
		break;   /* TRUE to enable alpha tests */
	case D3DRS_LASTPIXEL: 
		R_ASSERT(0);
		break;   /* TRUE for last-pixel on lines */
	case D3DRS_SRCBLEND: break;   /*case D3DBLEND */
	case D3DRS_DESTBLEND: break;   /*case D3DBLEND */
	case D3DRS_CULLMODE: break;   /*case D3DCULL */
	case D3DRS_ZFUNC: break;   /*case D3DCMPFUNC */
	case D3DRS_ALPHAREF: break;   /*case D3DFIXED */
	case D3DRS_ALPHAFUNC: break;   /*case D3DCMPFUNC */
	case D3DRS_DITHERENABLE: break;   /* TRUE to enable dithering */
	case D3DRS_ALPHABLENDENABLE: break;   /* TRUE to enable alpha blending */
	case D3DRS_FOGENABLE: break;   /* TRUE to enable fog blending */
	case D3DRS_SPECULARENABLE: break;   /* TRUE to enable specular */
	case D3DRS_FOGCOLOR: break;   /*case D3DCOLOR */
	case D3DRS_FOGTABLEMODE: break;   /*case D3DFOGMODE */
	case D3DRS_FOGSTART: break;   /* Fog start (for both vertex and pixel fog) */
	case D3DRS_FOGEND: break;   /* Fog end      */
	case D3DRS_FOGDENSITY: break;   /* Fog density  */
	case D3DRS_RANGEFOGENABLE: break;   /* Enables range-based fog */
	case D3DRS_STENCILENABLE: break;   /* BOOL enable/disable stenciling */
	case D3DRS_STENCILFAIL: break;   /*case D3DSTENCILOP to do if stencil test fails */
	case D3DRS_STENCILZFAIL: break;   /*case D3DSTENCILOP to do if stencil test passes and Z test fails */
	case D3DRS_STENCILPASS: break;   /*case D3DSTENCILOP to do if both stencil and Z tests pass */
	case D3DRS_STENCILFUNC: break;   /*case D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
	case D3DRS_STENCILREF: break;   /* Reference value used in stencil test */
	case D3DRS_STENCILMASK: break;   /* Mask value used in stencil test */
	case D3DRS_STENCILWRITEMASK: break;   /* Write mask applied to values written to stencil buffer */
	case D3DRS_TEXTUREFACTOR: break;   /*case D3DCOLOR used for multi-texture blend */
	case D3DRS_WRAP0: break;  /* wrap for 1st texture coord. set */
	case D3DRS_WRAP1: break;  /* wrap for 2nd texture coord. set */
	case D3DRS_WRAP2: break;  /* wrap for 3rd texture coord. set */
	case D3DRS_WRAP3: break;  /* wrap for 4th texture coord. set */
	case D3DRS_WRAP4: break;  /* wrap for 5th texture coord. set */
	case D3DRS_WRAP5: break;  /* wrap for 6th texture coord. set */
	case D3DRS_WRAP6: break;  /* wrap for 7th texture coord. set */
	case D3DRS_WRAP7: break;  /* wrap for 8th texture coord. set */
	case D3DRS_CLIPPING: break;
	case D3DRS_LIGHTING: break;
	case D3DRS_AMBIENT: break;
	case D3DRS_FOGVERTEXMODE: break;
	case D3DRS_COLORVERTEX: break;
	case D3DRS_LOCALVIEWER: break;
	case D3DRS_NORMALIZENORMALS: break;
	case D3DRS_DIFFUSEMATERIALSOURCE: break;
	case D3DRS_SPECULARMATERIALSOURCE: break;
	case D3DRS_AMBIENTMATERIALSOURCE: break;
	case D3DRS_EMISSIVEMATERIALSOURCE: break;
	case D3DRS_VERTEXBLEND: break;
	case D3DRS_CLIPPLANEENABLE: break;
	case D3DRS_POINTSIZE: break;   /* float point size */
	case D3DRS_POINTSIZE_MIN: break;   /* float point size min threshold */
	case D3DRS_POINTSPRITEENABLE: break;   /* BOOL point texture coord control */
	case D3DRS_POINTSCALEENABLE: break;   /* BOOL point size scale enable */
	case D3DRS_POINTSCALE_A: break;   /* float point attenuation A value */
	case D3DRS_POINTSCALE_B: break;   /* float point attenuation B value */
	case D3DRS_POINTSCALE_C: break;   /* float point attenuation C value */
	case D3DRS_MULTISAMPLEANTIALIAS: break;  // BOOL - set to do FSAA with multisample buffer
	case D3DRS_MULTISAMPLEMASK: break;  //case DWORD - per-sample enable/disable
	case D3DRS_PATCHEDGESTYLE: break;  // Sets whether patch edges will use float style tessellation
	case D3DRS_DEBUGMONITORTOKEN: break;  //case DEBUG ONLY - token to debug monitor
	case D3DRS_POINTSIZE_MAX: break;   /* float point size max threshold */
	case D3DRS_INDEXEDVERTEXBLENDENABLE: break;
	case D3DRS_COLORWRITEENABLE: break;  // per-channel write enable
	case D3DRS_TWEENFACTOR: break;   // float tween factor
	case D3DRS_BLENDOP: break;   //case D3DBLENDOP setting
	case D3DRS_POSITIONDEGREE: break;   // NPatch position interpolation degree.case D3DDEGREE_LINEAR orcase D3DDEGREE_CUBIC (default)
	case D3DRS_NORMALDEGREE: break;   // NPatch normal interpolation degree.case D3DDEGREE_LINEAR (default) orcase D3DDEGREE_QUADRATIC
	case D3DRS_SCISSORTESTENABLE: break;
	case D3DRS_SLOPESCALEDEPTHBIAS: break;
	case D3DRS_ANTIALIASEDLINEENABLE: break;
	case D3DRS_MINTESSELLATIONLEVEL: break;
	case D3DRS_MAXTESSELLATIONLEVEL: break;
	case D3DRS_ADAPTIVETESS_X: break;
	case D3DRS_ADAPTIVETESS_Y: break;
	case D3DRS_ADAPTIVETESS_Z: break;
	case D3DRS_ADAPTIVETESS_W: break;
	case D3DRS_ENABLEADAPTIVETESSELLATION: break;
	case D3DRS_TWOSIDEDSTENCILMODE: break;   /* BOOL enable/disable 2 sided stenciling */
	case D3DRS_CCW_STENCILFAIL: break;   /*case D3DSTENCILOP to do if ccw stencil test fails */
	case D3DRS_CCW_STENCILZFAIL: break;   /*case D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
	case D3DRS_CCW_STENCILPASS: break;   /*case D3DSTENCILOP to do if both ccw stencil and Z tests pass */
	case D3DRS_CCW_STENCILFUNC: break;   /*case D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
	case D3DRS_COLORWRITEENABLE1: break;   /* Additional ColorWriteEnables for the devices that supportcase D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
	case D3DRS_COLORWRITEENABLE2: break;   /* Additional ColorWriteEnables for the devices that supportcase D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
	case D3DRS_COLORWRITEENABLE3: break;   /* Additional ColorWriteEnables for the devices that supportcase D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
	case D3DRS_BLENDFACTOR: break;   /*case D3DCOLOR used for a constant blend factor during alpha blending for devices that supportcase D3DPBLENDCAPS_BLENDFACTOR */
	case D3DRS_SRGBWRITEENABLE: break;   /* Enable rendertarget writes to becase DE-linearized to SRGB (for formats that exposecase D3DUSAGE_QUERY_SRGBWRITE) */
	case D3DRS_DEPTHBIAS: break;
	case D3DRS_WRAP8: break;   /* Additional wrap states for vs_3_0+ attributes withcase D3DDECLUSAGE_TEXCOORD */
	case D3DRS_WRAP9: break;
	case D3DRS_WRAP10: break;
	case D3DRS_WRAP11: break;
	case D3DRS_WRAP12: break;
	case D3DRS_WRAP13: break;
	case D3DRS_WRAP14: break;
	case D3DRS_WRAP15: break;
	case D3DRS_SEPARATEALPHABLENDENABLE: break;  /* TRUE to enable a separate blending function for the alpha channel */
	case D3DRS_SRCBLENDALPHA: break;  /* SRC blend factor for the alpha channel whencase D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
	case D3DRS_DESTBLENDALPHA: break;  /*case DST blend factor for the alpha channel whencase D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
	case D3DRS_BLENDOPALPHA: break;  /* Blending operation for the alpha channel whencase D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

	default:
		break;
	}

	StateManager.Apply();
}

void CEditorRenderDevice::ReloadTextures()
{
	string_path Path = {};

	FS.update_path(Path, _game_textures_, "");
	FS.rescan_path(Path, true);

	Msg("* Reload textures...");
	UI->Resize();
}

void CEditorRenderDevice::UnloadTextures()
{
}

void CEditorRenderDevice::time_factor(float v)
{
	 Timer.time_factor(v);
	 TimerGlobal.time_factor(v);
}

void CEditorRenderDevice::CreateWindow()
{
	int DisplayX = GetSystemMetrics(SM_CXFULLSCREEN);
	int DisplayY = GetSystemMetrics(SM_CYFULLSCREEN);

	g_AppInfo.Window = SDL_CreateWindow("IX-Ray Editor", DisplayX, DisplayY, SDL_WINDOW_RESIZABLE);
}

void CEditorRenderDevice::DestryWindow()
{
	SDL_DestroyWindow(g_AppInfo.Window);
}