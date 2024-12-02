#include "stdafx.h"

HWND Platform::GetWindowHandle(SDL_Window* currentWindow) noexcept
{
	if (currentWindow == nullptr)
	{
		return nullptr;
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);

#if defined(SDL_VIDEO_DRIVER_X11)
	return (SDL_GetWindowWMInfo(currentWindow, &wmInfo) ? wmInfo.info.x11.window : nullptr);
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
	return (SDL_GetWindowWMInfo(currentWindow, &wmInfo) ? wmInfo.info.wl.surface : nullptr);
#elif defined(SDL_VIDEO_DRIVER_DIRECTFB)
	return (SDL_GetWindowWMInfo(currentWindow, &wmInfo) ? wmInfo.info.dfb.window : nullptr);
#elif defined(SDL_VIDEO_DRIVER_KMSDRM)
	// KMSDRM doesn't directly provide a window-like object; returning nullptr as a placeholder.
	// This case returns nullptr for now but can be expanded with a compatible solution.
	return nullptr;
#elif defined(SDL_VIDEO_DRIVER_VIVANTE)
	return (SDL_GetWindowWMInfo(currentWindow, &wmInfo) ? wmInfo.info.vivante.window : nullptr);
#else
	return nullptr; // Unsupported platform
#endif

}