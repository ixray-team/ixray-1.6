#include "stdafx.h"

extern float g_CameraJitterX;
extern float g_CameraJitterY;
extern float ps_r4_sharp_factor;
extern int ps_r4_sharp_enable;

void CRenderTarget::phase_dlss_combine()
{
    PIX_EVENT(DLSS);

    DLSSWrapper::DrawParameters dlssParams;
    dlssParams.deviceContext = HW.pContext;
    dlssParams.exposureResource = nullptr;
    dlssParams.unresolvedColorResource = rt_TargetCombined->pSurface;
    dlssParams.motionvectorResource = rt_MotionVectors->pSurface;
    dlssParams.depthbufferResource = rt_HWDepth->pSurface;
    dlssParams.reactiveMapResource = nullptr;
    dlssParams.transparencyAndCompositionResource = nullptr;
    dlssParams.resolvedColorResource = rt_UpscaleOutput->pSurface;
    dlssParams.renderWidth = RCache.get_width();
    dlssParams.renderHeight = RCache.get_height();
    dlssParams.cameraReset = false;
    dlssParams.cameraJitterX = g_CameraJitterX;
    dlssParams.cameraJitterY = g_CameraJitterY;
    dlssParams.frameTimeDelta = Device.fTrueTimeDelta * 1000.0f;
    dlssParams.nearPlane = VIEWPORT_NEAR;
    dlssParams.farPlane = g_pGamePersistent->Environment().CurrentEnv->far_plane;
    dlssParams.fovH = deg2rad(Device.fFOV);

    g_DLSSWrapper.Draw(dlssParams);
    HW.pContext->CopyResource(rt_Output->pSurface, rt_UpscaleOutput->pSurface);
}