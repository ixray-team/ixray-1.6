#include "stdafx.h"

HWND Platform::GetWindowHandle(SDL_Window* currentWindow) noexcept
{
	if (currentWindow == nullptr)
	{
		return nullptr;
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);

#if defined(SDL_VIDEO_DRIVER_WINDOWS)
	return (SDL_GetWindowWMInfo(currentWindow, &wmInfo) ? wmInfo.info.win.window : nullptr);
#elif defined(SDL_VIDEO_DRIVER_WINRT)
	// WinRT uses IInspectable*, which cannot directly translate to HWND.
	// This case returns nullptr for now but can be expanded with a compatible solution.
	return nullptr;
#else
	return nullptr; // Unsupported platform
#endif
}
