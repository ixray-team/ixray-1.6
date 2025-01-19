﻿#pragma once

#include "../../../xrEngine/device.h"
#include "UI_Camera.h"
#include "../../../Layers/xrRender/HWCaps.h"
#include "../../../Layers/xrRender/HW.h"
#include "../../../xrEngine/pure.h"
#include "../../../xrCore/FTimer.h"
#include "EStats.h"
#include "../../../xrEngine/Shader_xrLC.h"
#include "../../../Layers/xrRender/Shader.h"
#include "../../../Layers/xrRender/R_Backend.h"

//---------------------------------------------------------------------------
// refs
class CGameFont;
class CInifile;
class CResourceManager;
#undef CreateWindow
//------------------------------------------------------------------------------
class ECORE_API CEditorRenderDevice;
extern ECORE_API CEditorRenderDevice* EDevice;

#define REContext ((ID3D11DeviceContext*)Device.GetRenderContext())
#define REDevice ((ID3D11Device*)Device.GetRenderDevice())
#define RESwapchainTarget ((ID3D11RenderTargetView*)Device.GetSwapchainTexture())
#define RETarget ((ID3D11RenderTargetView*)Device.GetRenderTexture())
#define REDepth ((ID3D11DepthStencilView*)Device.GetDepthTexture())
#define RESwapchain ((IDXGISwapChain*)Device.GetSwapchain())

#define MAX_EDITOR_LIGHT 16

class ECORE_API CEditorRenderDevice :
	public CRenderDevice
{
	friend class CUI_Camera;
	friend class TUI;

private:
	float m_fNearer;

	ref_shader m_CurrentShader;

	void _SetupStates();
	void _Create(IReader* F);
	void _Destroy(BOOL bKeepTextures);

public:
	ref_shader m_WireShader;
	ref_shader m_WireShaderAxis;
	ref_shader m_SelectionShader;

	ref_texture texture_null;
	Fmaterial m_CurrentMat;
	Fmaterial m_DefaultMat;

	Flight m_Lights[MAX_EDITOR_LIGHT];
	BOOL m_EnableLights[MAX_EDITOR_LIGHT];

	ID3D11Buffer* m_MaterialBuffer;
	ID3D11Buffer* m_LightBuffer;

public:
	float RadiusRender;
	u32 dwRealWidth, dwRealHeight;
	float m_RenderArea;
	float m_ScreenQuality;

	u32 dwFillMode;
	u32 dwShadeMode;

	RECT NormalWinSize;
	bool NormalWinSizeSaved = false;
	bool isZoomed = false;
	//bool isMoving = false;
public:
	// camera
	CRegistrator<pureDrawUI> seqDrawUI;

	// Dependent classes
	CResourceManager* Resources;

public:
	CEditorRenderDevice();
	virtual ~CEditorRenderDevice();


	virtual bool Paused() const { return FALSE; };
	void time_factor(float);
	bool Create();
	void Destroy();
	void Resize(int w, int h, bool maximized);
	void ReloadTextures();
	void UnloadTextures();

	void RenderNearer(float f_Near);
	void ResetNearer();
	bool Begin();
	void End();

	void Initialize(void);
	void ShutDown(void);
	void Reset(IReader* F, BOOL bKeepTextures);

	void MaximizedWindow();
	void ResoreWindow(bool moving);
	void InitWindowStyle();

	virtual void DumpResourcesMemoryUsage()
	{
	}

	IC float GetRenderArea() { return m_RenderArea; }
	// Sprite rendering
	IC float _x2real(float x)
	{
		return (x + 1) * TargetWidth * 0.5f;
	}

	IC float _y2real(float y)
	{
		return (y + 1) * TargetHeight * 0.5f;
	}

	// draw
	void SetShader(ref_shader sh) { m_CurrentShader = sh; }
	ref_shader GetShader() { return m_CurrentShader; }
	void DP(D3DPRIMITIVETYPE pt, ref_geom geom, u32 startV, u32 pc);
	void DIP(D3DPRIMITIVETYPE pt, ref_geom geom, u32 baseV, u32 startV, u32 countV, u32 startI, u32 PC);

	void SetRS(D3DRENDERSTATETYPE p1, u32 p2);


	IC void SetSS(u32 sampler, D3DSAMPLERSTATETYPE type, u32 value)
	{
		VERIFY(b_is_Ready);

		R_ASSERT2(0, "Implement");

		//CHK_DX(REDevice->SetSamplerState(sampler,type,value));
	}

	// light&material
	IC void LightEnable(u32 dwLightIndex, BOOL bEnable)
	{
		m_EnableLights[dwLightIndex] = bEnable;
	}

	IC void SetLight(u32 dwLightIndex, Flight& lpLight)
	{
		m_Lights[dwLightIndex] = lpLight;
	}

	IC void SetMaterial(Fmaterial& mat)
	{
		m_CurrentMat = mat;
	}

	IC void ResetMaterial()
	{
		m_CurrentMat = m_DefaultMat;
	}

	void UpdateMaterial();
	void UpdateLight();

	// update
	void UpdateView();
	void FrameMove();

	bool MakeScreenshot(U32Vec& pixels, u32 width, u32 height);


	void InitTimer();

	// Mode control
	virtual void Pause(BOOL bOn, BOOL bTimer, BOOL bSound, LPCSTR reason) override
	{
	}

	virtual void PreCache(u32 amount, bool b_draw_loadscreen, bool b_wait_user_input) override
	{
	}

	virtual void Clear();

public:
	Shader_xrLC_LIB ShaderXRLC;

private:
	virtual void _BCL AddSeqFrame(pureFrame* f, bool mt);
	virtual void _BCL RemoveSeqFrame(pureFrame* f);

private:
	HWND hwnd;
public:
	HWND GetHWND() { return hwnd; }
	void CreateWindow();
	void DestryWindow();
	virtual void Reset(bool precache);
	virtual bool IsEditorMode() override { return true; }
};

// video
enum
{
	rsFilterLinear = (1ul << 20ul),
	rsEdgedFaces = (1ul << 21ul),
	rsRenderTextures = (1ul << 22ul),
	rsFog = (1ul << 24ul),
	rsRenderRealTime = (1ul << 25ul),
	rsDrawGrid = (1ul << 26ul),
	rsDrawSafeRect = (1ul << 27ul),
	rsMuteSounds = (1ul << 28ul),
	rsEnvironment = (1ul << 29ul),
	rsDrawAxis = (1ul << 30ul),
	rsDisableAxisCube = (1ul << 31ul),
};

#define DEFAULT_CLEARCOLOR 0x00555555

#define REQ_CREATE()	if (!EDevice->bReady)	return;
#define REQ_DESTROY()	if (EDevice->bReady)	return;

#include "../../../Layers/xrRender/R_Backend_Runtime.h"
